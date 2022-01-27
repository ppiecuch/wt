#ifndef DB_USER
#define DB_USER

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

#include <string>

using namespace Wt;

namespace dbo = Wt::Dbo;

class User;

typedef Auth::Dbo::AuthInfo<User> AuthInfo;
typedef dbo::collection< dbo::ptr<User> > Users;

class User {
public:
  std::string name; /* a copy of auth info's user name */
  int gamesPlayed;
  long long score;
  WDateTime lastGame;
  dbo::collection<dbo::ptr<AuthInfo>> authInfos;

  template<class Action>
  void persist(Action& a) {
    dbo::field(a, gamesPlayed, "gamesPlayed");
    dbo::field(a, score, "score");
    dbo::field(a, lastGame, "lastGame");
    dbo::hasMany(a, authInfos, dbo::ManyToOne, "user");
  }

  User();
};

DBO_EXTERN_TEMPLATES(User);

#endif // DB_USER
