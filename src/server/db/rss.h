#ifndef DB_RSS
#define DB_RSS

#include <Wt/WDateTime.h>

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

#include <string>

namespace dbo = Wt::Dbo;

class Rss {
public:
  WString title;
  WString brief;
  WString body;
  WDateTime date;

  std::string permaLink() const;

  template<class Action>
  void persist(Action& a) {
    dbo::field(a, date, "date");
    dbo::field(a, title, "title");
    dbo::field(a, body, "body");
  }

  Rss();
  Rss(const WString &title, const WString &body, const WDateTime &date);
};

typedef dbo::collection<dbo::ptr<Rss>> Journal;

DBO_EXTERN_TEMPLATES(Rss)

#endif // DB_RSS
