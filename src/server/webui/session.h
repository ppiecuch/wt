#ifndef WEBUI_SESSION
#define WEBUI_SESSION

#include <Wt/Auth/Login.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>

#include <memory>
#include <vector>

#include "../db/leaderboard.h"

using namespace Wt;

typedef Auth::Dbo::UserDatabase<AuthInfo> AuthUserDatabase;

class Session : public Dbo::Session {
  Dbo::SqlConnectionPool *connectionPool_;
  std::unique_ptr<AuthUserDatabase> users_;
  Auth::Login login_;

  DboPlayer player() const;
  DboDevice device() const;

public:
  static void configureAuth();

  Dbo::Session &dbo() { return *this; }

  Auth::AbstractUserDatabase &users();
  Auth::Login &login() { return login_; }

  std::vector<Player> topPlayers(int limit);

  // These methods deal with the currently logged in user
  std::string playerName() const;
  int findRanking();
  void addToScore(int s);

  static const Auth::AuthService &auth();
  static const Auth::AbstractPasswordService &passwordAuth();
  static std::unique_ptr<Dbo::SqlConnectionPool> createConnectionPool(std::string &connection_string);
  static std::string getDatabaseFilename(const std::string &connection_string) {
      if (strncmp(connection_string.c_str(), "sq://", 5) == 0) {
        return connection_string.substr(5);
      }
      return "";
  }

  Session(Dbo::SqlConnectionPool *connectionPool);
  ~Session();
};

#endif // WEBUI_SESSION
