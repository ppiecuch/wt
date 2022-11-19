#include "session.h"

#include "Wt/Auth/Identity.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/PasswordStrengthValidator.h"
#include "Wt/Auth/PasswordVerifier.h"
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

#include "../common/utils.h"

namespace {

#ifdef HAVE_CRYPT
  class UnixCryptHashFunction : public Auth::HashFunction {
  public:
    virtual std::string compute(const std::string& msg, const std::string& salt) const {
      std::string md5Salt = "$1$" + salt;
      return crypt(msg.c_str(), md5Salt.c_str());
    }

    virtual bool verify(const std::string& msg, const std::string& salt, const std::string& hash) const {
      return crypt(msg.c_str(), hash.c_str()) == hash;
    }

    virtual std::string name () const { return "crypt"; }
  };
#endif // HAVE_CRYPT

  Auth::AuthService authService;
  Auth::PasswordService passwordService(authService);
} // namespace

void Session::configureAuth() {
  authService.setAuthTokensEnabled(true, "servercookie");
  authService.setEmailVerificationEnabled(true);
  authService.setEmailVerificationRequired(true);

  std::unique_ptr<Auth::PasswordVerifier> verifier = std::make_unique<Auth::PasswordVerifier>();
  verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));

#ifdef HAVE_CRYPT
  // Support for users registered in the pre - Wt::Auth version
  verifier->addHashFunction(std::make_unique<UnixCryptHashFunction>());
#endif

  passwordService.setVerifier(std::move(verifier));
  passwordService.setStrengthValidator(std::make_unique<Auth::PasswordStrengthValidator>());
  passwordService.setAttemptThrottlingEnabled(true);
}

std::unique_ptr<Dbo::SqlConnectionPool> Session::createConnectionPool(std::string &connection_string) {
  static const char *_backends[] = {
#ifdef DEBUG
    "sq://server-debug.db"
    "pq://127.0.0.1",
#else
    "pq://127.0.0.1",
    "sq://server.db",
#endif
    nullptr
  };

  connection_string = "";

  for (auto conn = _backends; *conn; conn++) {
    if (strncmp(*conn, "pq://", 5) == 0) {
#ifdef DBO_POSTGRES
      auto result = std::make_unique<Dbo::backend::Postgres>(*conn+5);
      if (result->connection() != nullptr) {
        result->setDateTimeStorage(Dbo::SqlDateTimeType::DateTime, Dbo::backend::DateTimeStorage::PseudoISO8601AsText);
#ifdef DEBUG
      result->setProperty("show-queries", "true");
#endif
      connection_string = *conn;
      return std::make_unique<Dbo::FixedSqlConnectionPool>(std::move(result), 10);
      }
#endif
    } else if (strncmp(*conn, "sq://", 5) == 0) {
      auto result = std::make_unique<Dbo::backend::Sqlite3>(*conn+5);
      if (result->connection() != nullptr) {
        result->setDateTimeStorage(Dbo::SqlDateTimeType::DateTime, Dbo::backend::DateTimeStorage::PseudoISO8601AsText);
#ifdef DEBUG
        result->setProperty("show-queries", "true");
#endif
        connection_string = *conn;
        return std::make_unique<Dbo::FixedSqlConnectionPool>(std::move(result), 5);
      }
    } else
      LogInfo(std::string("Unknow connection string: ") + *conn);
  }
  throw std::runtime_error("Unable to open database");
}

Session::Session(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) {
  setConnectionPool(*connectionPool_);

  users_ = std::make_unique<AuthUserDatabase>(dbo());
}

Session::~Session() { }

DboPlayer Session::player() const {
  if (login_.loggedIn()) {
    DboAuthInfo authInfo = users_->find(login_.user());
    DboPlayer user = authInfo->user();

    if (!user) {
      user = const_cast<Session*>(this)->add(std::make_unique<Player>());
      authInfo.modify()->setUser(user);
    }

    return user;
  } else
    return DboPlayer();
}

std::string Session::playerName() const {
  if (login_.loggedIn())
    return login_.user().identity(Auth::Identity::LoginName).toUTF8();
  else
    return std::string();
}

void Session::addToScore(int s) {
  Dbo::Transaction transaction(dbo());

  DboPlayer u = player();
  if (u) {
    u.modify()->score += s;
    ++u.modify()->gamesPlayed;
    u.modify()->lastGame = WDateTime::currentDateTime();
  }

  transaction.commit();
}

std::vector<Player> Session::topPlayers(int limit) {
  Dbo::Transaction transaction(dbo());

  DboPlayers top = find<Player>().orderBy("score desc").limit(limit);

  std::vector<Player> result;
  for (DboPlayers::const_iterator i = top.begin(); i != top.end(); ++i) {
    DboPlayer user = *i;
    result.push_back(*user);
  }

  transaction.commit();

  return result;
}

int Session::findRanking() {
  Dbo::Transaction transaction(dbo());
  
  DboPlayer u = player();
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
