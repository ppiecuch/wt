#include <Wt/Dbo/Impl.h>
#include <Wt/WWebWidget.h>

#include "rss.h"

DBO_INSTANTIATE_TEMPLATES(Rss)

std::string Rss::permaLink() const { return ""; }

Rss::Rss() { }

Rss::Rss(const WString &t, const WString &b, const Wt::WDateTime &d)
  : title(t)
  , body(b)
  , date(d)
{ }
