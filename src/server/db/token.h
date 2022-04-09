#ifndef DB_TOKEN
#define DB_TOKEN

#include <Wt/WDateTime.h>
#include <Wt/Auth/Token.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include <string>

using namespace Wt;
namespace dbo = Wt::Dbo;

class User;

class Token : public dbo::Dbo<Auth::Token> {
public:
  dbo::ptr<User> user;

  std::string value;
  WDateTime expires;

  template<class Action>
  void persist(Action &a) {
    dbo::field(a, value,   "value");
    dbo::field(a, expires, "expires");

    dbo::belongsTo(a, user, "user");
  }

  Token();
  Token(const std::string &value, const WDateTime &expires);
};

DBO_EXTERN_TEMPLATES(Token)

#endif // DB_TOKEN
