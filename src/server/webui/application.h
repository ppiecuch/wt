#ifndef WEBUI_APPLICATION
#define WEBUI_APPLICATION

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <Wt/WApplication.h>
#include <Wt/WLogger.h>

#include <string>

namespace Wt {
  class WLineEdit;
  class WText;
  class WContainerWidget;
}

using namespace Wt;

class GameServerApplication : public WApplication {
public:
  GameServerApplication(const WEnvironment& env);

private:
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

  void showLogin();
  void loginHandler();

  void showLandingPage();

  void logout() {
    loginDone = false;
    user = "";
    pass = "";
    quit();
  }

  void handleQuote();
  void handlePf();
  void handleTrx();
  void handleBuy();
  void handleSell();
};

int app_landing(int argc, char **argv);

#endif // WEBUI_APPLICATION
