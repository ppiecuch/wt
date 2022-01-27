#ifndef WEBUI_APPLICATION
#define WEBUI_APPLICATION

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <Wt/WApplication.h>
#include <Wt/WLogger.h>

#include <string>

#include "session.h"

namespace Wt {
  class WLineEdit;
  class WText;
  class WContainerWidget;
}

using namespace Wt;

class SysMon;

class GameServerApplication : public WApplication {

  WLineEdit* nameEdit_;
  WLineEdit* passEdit_;
  WText* message_;
  WLineEdit* stockCode;

  WLineEdit* bsc;
  WLineEdit* bqt;
  WLineEdit* bpr;

  WLineEdit* ssc;
  WLineEdit* sqt;
  WLineEdit* spr;

  WContainerWidget* center;

  std::string user;
  std::string pass;

  bool loginDone;

  void loginHandler();
  void handleInternalPath(const std::string &internalPath);

  void showLogin();
  void showLandingPage();
  void showSysmon();

  void logout() {
    loginDone = false;
    user = "";
    pass = "";
    quit();
  }

  std::unique_ptr<SysMon> sysmon;

  void handleQuote();
  void handlePf();
  void handleTrx();
  void handleBuy();
  void handleSell();

public:
  GameServerApplication(const WEnvironment& env);
};

int app_landing(int argc, char **argv);

#endif // WEBUI_APPLICATION
