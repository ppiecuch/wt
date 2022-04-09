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

  DboUser user() const;

public:
  static void configureAuth();

  Dbo::Session &dbo() { return *this; }

  Auth::AbstractUserDatabase &users();
  Auth::Login &login() { return login_; }

  std::vector<User> topUsers(int limit);

  // These methods deal with the currently logged in user
  std::string userName() const;
  int findRanking();
  void addToScore(int s);

  static const Auth::AuthService &auth();
  static const Auth::AbstractPasswordService &passwordAuth();
  static std::unique_ptr<Dbo::SqlConnectionPool> createConnectionPool();

  Session(Dbo::SqlConnectionPool *connectionPool);
  ~Session();
};

#endif //WEBUI_SESSION
