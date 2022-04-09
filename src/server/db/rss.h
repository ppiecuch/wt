#ifndef DB_RSS
#define DB_RSS

#include <Wt/WDateTime.h>

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

#include <string>

class Rss {
public:
  WString topic;
  WString title;
  WString brief;
  WString body;
  WDateTime date;

  std::string permaLink() const { return ""; }

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, date, "date");
    Dbo::field(a, title, "topic");
    Dbo::field(a, title, "title");
    Dbo::field(a, brief, "brief");
    Dbo::field(a, body, "body");
  }

  Rss() { }
  Rss(const WString &topic, const WString &title, const WString &body, const WDateTime &date)
    : topic(topic), title(title), body(body), date(date) {  }
};

typedef Dbo::collection<Dbo::ptr<Rss>> Journal;

DBO_EXTERN_TEMPLATES(Rss)

#endif // DB_RSS
