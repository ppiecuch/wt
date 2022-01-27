#ifndef DB_TOKEN
#define DB_TOKEN

#include <Wt/WDate.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

#include <string>

class User;

class Token : public Wt::Dbo::Dbo<Token> {
public:
  Wt::Dbo::ptr<User> user;

  std::string value;
  WDateTime expires;

  template<class Action>
  void persist(Action &a) {
    Wt::Dbo::field(a, value,   "value");
    Wt::Dbo::field(a, expires, "expires");

    Wt::Dbo::belongsTo(a, user, "user");
  }

  Token();
  Token(const std::string &value, const WDateTime &expires);
};

DBO_EXTERN_TEMPLATES(Token)

#endif // DB_TOKEN
