#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <Wt/WServer.h>
#include <Wt/WEnvironment.h>

#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WBorderLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WToolBar.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>

using namespace Wt;

#include "application.h"
#include "session.h"

#include "../common/utils.h"
#include "../common/controller.h"
#include "../sysmon/sysmon.h"
#include "../rest/resources.h"
#include "../rest/rssfeed.h"
#include "../db/leaderboard.h"
#include "../db/rss.h"

#include <tuple>
#include <functional>
#include <iostream>

#include <boost/filesystem.hpp>

GameServerApplication::GameServerApplication(const WEnvironment& env)
  : WApplication(env)
  , user(""), pass("")
  , loginDone(false)
  , sysmon(std::make_unique<SysMon>())
{
  showLogin();
}

void GameServerApplication::handleInternalPath(const std::string &internalPath) {
}

void GameServerApplication::showLogin() {

  setTitle("Game Server : Login");

  auto vertDummy = root()->addWidget(std::make_unique<WContainerWidget>());
  vertDummy->resize(275, 275);
  root()->addWidget(std::make_unique<WBreak>());
  auto loginBoxContainer = root()->addWidget(std::make_unique<WContainerWidget>());
  loginBoxContainer->resize(200, 200);

  root()->setContentAlignment(Wt::AlignmentFlag::Center);

  auto outerVB = loginBoxContainer->setLayout(std::make_unique<WVBoxLayout>());

  auto userRow = outerVB->addLayout(std::make_unique<WHBoxLayout>());
  auto passRow = outerVB->addLayout(std::make_unique<WHBoxLayout>());
  auto buttonRow = outerVB->addLayout(std::make_unique<WHBoxLayout>());

  userRow->addWidget(std::make_unique<WText>("User name"));
  nameEdit_ = userRow->addWidget(std::make_unique<WLineEdit>());
  nameEdit_->setFocus();

  passRow->addWidget(std::make_unique<WText>("Password"));
  passEdit_ = passRow->addWidget(std::make_unique<WLineEdit>());

  auto loginButton = buttonRow->addWidget(std::make_unique<WPushButton>("Login"));
  auto clearButton = buttonRow->addWidget(std::make_unique<WPushButton>("Clear"));
  auto regButton = buttonRow->addWidget(std::make_unique<WPushButton>("Register"));

  auto reg_msg_row = outerVB->addLayout(std::make_unique<WHBoxLayout>());
  message_ = reg_msg_row->addWidget(std::make_unique<WText>("Login with credentials"));

  auto bound_login = std::bind(&GameServerApplication::loginHandler, this);
  loginButton->clicked().connect(bound_login);
  passEdit_->enterPressed().connect(bound_login);

  nameEdit_->enterPressed().connect(std::bind([=]() {
      passEdit_->setFocus();
  }));

  clearButton->clicked().connect(std::bind([=]() {
    nameEdit_->setText("");
    passEdit_->setText("");
  }));

  regButton->clicked().connect(std::bind([=]() {
    CommonController cc;
    auto user = nameEdit_->text();
    auto pass = passEdit_->text();
    OperationStatus op_stat = cc.registerNewTrader(user.toUTF8(), pass.toUTF8());
    if (op_stat.failed()) {
      std::string err = op_stat.get_exception_msg();
      message_->setText(std::string("Registration failed: ") + err);
    } else {
      root()->clear();
      showLandingPage();
    }
  }));
}

void GameServerApplication::showSysmon() {
  setTitle("Game Server : Monitor");

  auto container = root()->addWidget(std::make_unique<WContainerWidget>());
  container->setId("SysMonPage");
  container->addWidget(sysmon->layout());
  root()->setContentAlignment(Wt::AlignmentFlag::Center);
}

void GameServerApplication::showLandingPage() {
  auto container = root()->addWidget(std::make_unique<WContainerWidget>());
  container->resize(800, 800);

  auto layout = container->setLayout(std::make_unique<WBorderLayout>());

  auto northItem = layout->addWidget(std::make_unique<WContainerWidget>(), Wt::LayoutPosition::North);
  auto northBox = northItem->setLayout(std::make_unique<WHBoxLayout>());
  auto item = northBox->addWidget(std::make_unique<WText>("Main Landing page : Perform operations using toolbar and center form"));
  auto logoutButton = northBox->addWidget(std::make_unique<WPushButton>("Sign out"));

  auto tbContainer = layout->addWidget(std::make_unique<WContainerWidget>(), Wt::LayoutPosition::West);
  tbContainer->resize(100, 600);

  auto toolBar = tbContainer->addWidget(std::make_unique<WToolBar>());

  WPushButton *quoteButton = toolBar->addButton(std::make_unique<WPushButton>("Get Quote"));
  toolBar->addSeparator();

  WPushButton *pfButton = toolBar->addButton(std::make_unique<WPushButton>("View Portfolio"));
  toolBar->addSeparator();

  WPushButton *trxButton = toolBar->addButton(std::make_unique<WPushButton>("View Transactions"));
  toolBar->addSeparator();

  WPushButton *buyButton = toolBar->addButton(std::make_unique<WPushButton>("Buy"));
  toolBar->addSeparator();

  WPushButton *sellButton = toolBar->addButton(std::make_unique<WPushButton>("Sell"));

  toolBar->setOrientation(Wt::Orientation::Horizontal);

  center = layout->addWidget(std::make_unique<WContainerWidget>(), Wt::LayoutPosition::Center);
  center->addWidget(std::make_unique<WText>("Work space in Center"));

  logoutButton->clicked().connect(std::bind([=]() {
    logout();
  }));

  quoteButton->clicked().connect(std::bind([=]() {
    center->clear();
    handleQuote();
  }));

  pfButton->clicked().connect(std::bind([=]() {
    center->clear();
    handlePf();
  }));

  trxButton->clicked().connect(std::bind([=]() {
    center->clear();
    handleTrx();
  }));

  buyButton->clicked().connect(std::bind([=]() {
    center->clear();
    handleBuy();
  }));

  sellButton->clicked().connect(std::bind([=]() {
    center->clear();
    handleSell();
  }));
}

void GameServerApplication::loginHandler() {
  CommonController cc;
  OperationStatus op_stat;
  auto username = nameEdit_->text();
  auto password = passEdit_->text();

  if (cc.areValidCredentials(username.toUTF8(), password.toUTF8(), op_stat)) {
    loginDone = true;
    user = username.toUTF8();
    pass = password.toUTF8();
    root()->clear();
    showLandingPage();
  } else {
    loginDone = false;
    message_->setText("Login failed. Try again.");
  }
}

void GameServerApplication::handleBuy() { }
void GameServerApplication::handleSell() { }
void GameServerApplication::handleQuote() { }
void GameServerApplication::handlePf() { }
void GameServerApplication::handleTrx() { }

inline static void report_exception(const char *what) {
  std::cerr << "> Server exception:"
    << std::endl
    << "> -----------------"
    << std::endl
    << what
    << std::endl
    << "> -----------------"
    << std::endl;
}

std::unique_ptr<Wt::WApplication> createApplication(const WEnvironment& env, Dbo::SqlConnectionPool *connectionPool) {
  std::cerr << "> Creating application for IP " << env.clientAddress() << std::endl;
  return std::make_unique<GameServerApplication>(env);
}

int app_landing(int argc, char **argv) {
  try {
    WServer server(argv[0], "");
    try {
      server.setServerConfiguration(argc, argv);

      Session::configureAuth();

      std::string _connection_string;
      std::unique_ptr<Dbo::SqlConnectionPool> db = Session::createConnectionPool(_connection_string);

      {
        Dbo::Session session;
        session.setConnectionPool(*db);

        session.mapClass<DeviceToken>("device_tokens");
        session.mapClass<Device>("device");
        session.mapClass<Team>("team");
        session.mapClass<Player>("player");
        session.mapClass<AuthInfo>("auth_info");
        session.mapClass<AuthInfo::AuthIdentityType>("auth_identity");
        session.mapClass<AuthInfo::AuthTokenType>("auth_token");
        session.mapClass<Game>("game");
        session.mapClass<Ranking>("ranking");
        session.mapClass<Score>("score");
        session.mapClass<Rss>("rss");

        try {
          std::string dbfile = Session::getDatabaseFilename(_connection_string);
          if (!dbfile.empty() && boost::filesystem::exists(dbfile) && boost::filesystem::file_size(dbfile) > 0) {
            log("info") << "(Server) Database already exists: " << _connection_string << "(" << boost::filesystem::file_size(dbfile) << " bytes )";
          } else {
            session.createTables();
            log("info") << "(Server) Database created";
          }
        } catch (Dbo::Exception &e) {
          log("info") << "(Server) Database failed when initializing: " << _connection_string;
          report_exception(e.what());
          return 1;
        } catch (std::exception &e) {
          log("info") << "(Server) Database initialization trigger unexpected exception " << e.what();
        }
      }

      ApiServiceRegistering registering(db.get());
      ApiServiceScore score(db.get());
      ApiServiceRanking ranking(db.get());

      // POST /api/device/register -> token (optional body: email address, display name)
      // {
      //   "device-name": "ios-xperia-1AC9D"
      //   "team-id": "7A83"
      // }
      //  result:
      //    err: already registered
      //    ok: token

      // POST /api/device/rename -> token (optional body: email address, display name)

      // POST /api/device/attach -> attach device to the player
      // {
      //   "player-id": "AB07"
      // }
      //  result:
      //    err: player not found
      //    ok: device attached

      // POST /api/device/ping

      // GET /api/device/status/ios-xperia-1AC9D -> token (optional body: email address, display name)
      //  result:
      //    status: already registered
      //    status: unknown device

      // POST /api/device/deregister/ios-xperia-1AC9D -> remove token if exists
      //  Header:
      //    Authentication: <token>
      //  result:
      //    err: token not exists
      //    status: token removed
      // GET /api/mysticmine/retrive/<email> -> token send to registered email

      // GET /api/ranking/mysticmine/normal/global/10
      // UPDATE /api/ranking/mysticmine/normal/ppiecuch/1234

      //  Header:
      //    Authentication: <token>

      server.addResource(&registering, "/api/device/${action}");
      server.addResource(&registering, "/api/register/${game}/${pin}");
      server.addResource(&score, "/api/score/${game}/${ranking}/${score}");
      // get $top items from $chart
      server.addResource(&ranking, "/api/ranking/${game}/${ranking}/${chart}/${top}");

      RSSFeed rss(db.get(), "KomSoft GameServer journal", "https://service-gate.in.net", "Latest events.");
      // GET "/rss/mysticmine"
      //  all entries for selected topic
      // GET "/rss/all/last/20"
      //  last 20 entries for given topic
      // GET "/rss/mysticmine/from/1234565"
      //  entries from given timestamp

      server.addResource(&rss, "/feed/${action}/${arg}");
      server.addResource(&rss, "/feed/${topic}/${action}/${arg}");

      server.addEntryPoint(EntryPointType::Application,
                           std::bind(&createApplication, std::placeholders::_1, db.get()));

      if (server.start()) {
        WServer::waitForShutdown();
        server.stop();
      } else {
        return 1;
      }
      return 0;
    } catch (std::exception& e) {
      report_exception(e.what());
      return 1;
    }
  } catch (WServer::Exception& e) {
    report_exception(e.what());
    return 1;
  } catch (std::exception& e) {
    report_exception(e.what());
    return 1;
  } catch (...) {
    std::cerr << "Unknown server exception" << std::endl;
    return 1;
  }
  return 0;
}
