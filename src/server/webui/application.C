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

#include "../common/utils.h"
#include "../common/controller.h"
#include "../sysmon/sysmon.h"
#include "../rest/resources.h"
#include "../rest/rssfeed.h"

#include <tuple>
#include <functional>
#include <iostream>

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

      std::unique_ptr<Dbo::SqlConnectionPool> db = Session::createConnectionPool();

      ApiService api(db.get());
      server.addResource(&api, "/api");

      // GET /api/mysticmine/register/ppiecuch -> token
      // GET /api/mysticmine/retrive/<email> -> token

      // GET /api/mysticmine/normal/top/10
      // UPDATE /api/mysticmine/normal/ppiecuch/<token>/1234

      server.addResource(&api, "/api/${game}/${subindex}/${action}/${arg}");

      RSSFeed rss(db.get(), "Game Server journal", "", "Game Server latest events.");
      server.addResource(&rss, "/rss");
      server.addResource(&rss, "/rss/last/${num}");
      server.addResource(&rss, "/rss/from/${timestamp}");

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
