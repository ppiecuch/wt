#include "session.h"

#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/PasswordStrengthValidator.h"
#include "Wt/Auth/PasswordVerifier.h"
#include "Wt/Auth/GoogleService.h"
#include "Wt/Auth/Dbo/AuthInfo.h"
#include "Wt/Auth/Dbo/UserDatabase.h"
#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include <Wt/WApplication.h>
#include <Wt/WLogger.h>

using namespace Wt;

#ifndef WT_WIN32
# include <unistd.h>
#endif

#if !defined(WT_WIN32) && !defined(__CYGWIN__) && !defined(ANDROID)
#define HAVE_CRYPT
#ifndef _XOPEN_CRYPT
# include <crypt.h>
#endif // _XOPEN_CRYPT
#endif

namespace {

#ifdef HAVE_CRYPT
  class UnixCryptHashFunction : public Auth::HashFunction {
  public:
    virtual std::string compute(const std::string& msg,
                                const std::string& salt) const
    {
      std::string md5Salt = "$1$" + salt;
      return crypt(msg.c_str(), md5Salt.c_str());
    }

    virtual bool verify(const std::string& msg,
                        const std::string& salt,
                        const std::string& hash) const
    {
      return crypt(msg.c_str(), hash.c_str()) == hash;
    }

    virtual std::string name () const { return "crypt"; }
  };
#endif // HAVE_CRYPT

  class OAuth : public std::vector<const Auth::OAuthService *> {
  public:
    ~OAuth() {
      for (unsigned i = 0; i < size(); ++i)
        delete (*this)[i];
    }
  };

  Auth::AuthService authService;
  Auth::PasswordService passwordService(authService);
  OAuth authServices;
} // namespace

void Session::configureAuth() {
  authService.setAuthTokensEnabled(true, "servercookie");
  authService.setEmailVerificationEnabled(true);

  std::unique_ptr<Auth::PasswordVerifier> verifier = std::make_unique<Auth::PasswordVerifier>();
  verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));

#ifdef HAVE_CRYPT
  // Support for users registered in the pre - Wt::Auth version
  verifier->addHashFunction(std::make_unique<UnixCryptHashFunction>());
#endif

  passwordService.setVerifier(std::move(verifier));
  passwordService.setStrengthValidator(std::make_unique<Auth::PasswordStrengthValidator>());
  passwordService.setAttemptThrottlingEnabled(true);

  if (Auth::GoogleService::configured())
    authServices.push_back(new Auth::GoogleService(authService));
}

std::unique_ptr<Dbo::SqlConnectionPool> Session::createConnectionPool() {
  auto connection = std::make_unique<Dbo::backend::Sqlite3>("server.db");
#ifdef DEBUG
  connection->setProperty("show-queries", "true");
#endif
  connection->setDateTimeStorage(Dbo::SqlDateTimeType::DateTime, Dbo::backend::DateTimeStorage::PseudoISO8601AsText);

  return std::make_unique<Dbo::FixedSqlConnectionPool>(std::move(connection), 10);
}

Session::Session(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) {
  setConnectionPool(*connectionPool_);

  mapClass<User>("user");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");

  users_ = std::make_unique<UserDatabase>(dbo());

  Dbo::Transaction transaction(dbo());
  try {
    createTables();

    // Add a default guest/guest account
    Auth::User guestUser = users_->registerNew();
    guestUser.addIdentity(Auth::Identity::LoginName, "guest");
    passwordService.updatePassword(guestUser, "guest");

    log("info") << "Database created";
  } catch (...) {
    log("info") << "Using existing database";
  }

  transaction.commit();
}

Session::~Session() { }

Dbo::ptr<User> Session::user() const {
  if (login_.loggedIn()) {
    Dbo::ptr<AuthInfo> authInfo = users_->find(login_.user());
    Dbo::ptr<User> user = authInfo->user();

    if (!user) {
      user = const_cast<Session*>(this)->add(std::make_unique<User>());
      authInfo.modify()->setUser(user);
    }

    return user;
  } else
    return Dbo::ptr<User>();
}

std::string Session::userName() const {
  if (login_.loggedIn())
    return login_.user().identity(Auth::Identity::LoginName).toUTF8();
  else
    return std::string();
}

void Session::addToScore(int s) {
  Dbo::Transaction transaction(dbo());

  Dbo::ptr<User> u = user();
  if (u) {
    u.modify()->score += s;
    ++u.modify()->gamesPlayed;
    u.modify()->lastGame = WDateTime::currentDateTime();
  }

  transaction.commit();
}

std::vector<User> Session::topUsers(int limit) {
  Dbo::Transaction transaction(dbo());

  Users top = find<User>().orderBy("score desc").limit(limit);

  std::vector<User> result;
  for (Users::const_iterator i = top.begin(); i != top.end(); ++i) {
    Dbo::ptr<User> user = *i;
    result.push_back(*user);

    Dbo::ptr<AuthInfo> auth = *user->authInfos.begin();
    std::string name = auth->identity(Auth::Identity::LoginName).toUTF8();

    result.back().name = name;
  }

  transaction.commit();

  return result;
}

int Session::findRanking() {
  Dbo::Transaction transaction(dbo());
  
  Dbo::ptr<User> u = user();
  int ranking = -1;

  if (u)
    ranking = query<int>("select distinct count(score) from user")
      .where("score > ?").bind(u->score);

  transaction.commit();
  
  return ranking + 1;
}

Auth::AbstractUserDatabase& Session::users() { return *users_; }

const Auth::AuthService& Session::auth() { return authService; }

const Auth::AbstractPasswordService& Session::passwordAuth() { return passwordService; }

const std::vector<const Auth::OAuthService *>& Session::oAuth() { return authServices; }
