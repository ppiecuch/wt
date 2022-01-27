#ifndef DB_TOKEN
#define DB_TOKEN

#include <Wt/WDate.h>

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include <string>

class User;

namespace dbo = Wt::Dbo;

class Token : public dbo::Dbo<Token> {
public:
  dbo::ptr<User> user;

  std::string    value;
  Wt::WDateTime  expires;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, value,   "value");
    dbo::field(a, expires, "expires");

    dbo::belongsTo(a, user, "user");
  }

  Token();
  Token(const std::string& value, const Wt::WDateTime& expires);
};

DBO_EXTERN_TEMPLATES(Token)

#endif // DB_TOKEN
