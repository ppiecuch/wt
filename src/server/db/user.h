#ifndef DB_USER
#define DB_USER

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

#include <string>

using namespace Wt;

class User;

typedef Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Dbo::collection<Dbo::ptr<User>> Users;

class User {
public:
  std::string name; /* a copy of auth info's user name */
  int gamesPlayed;
  long long score;
  WDateTime lastGame;
  Dbo::collection<Dbo::ptr<AuthInfo>> authInfos;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, gamesPlayed, "gamesPlayed");
    Dbo::field(a, score, "score");
    Dbo::field(a, lastGame, "lastGame");
    Dbo::hasMany(a, authInfos, Dbo::ManyToOne, "user");
  }

  User();
};

DBO_EXTERN_TEMPLATES(User);

#endif // DB_USER
