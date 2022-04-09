#include <Wt/Http/Response.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Utils.h>

using namespace Wt;

#include "../db/rss.h"

#include "rssfeed.h"

static WDateTime _fromTime(time_t tm) {
  WDateTime t;
  t.setTime_t(tm);
  return t;
}

void RSSFeed::handleRequest(const Http::Request &request, Http::Response &response) {
  Dbo::Session session;
  session.setConnectionPool(*connectionPool_);

  session.mapClass<Rss>("rss");

  response.setMimeType("application/rss+xml");

  std::string url = url_;

  if (url.empty()) {
    url = request.urlScheme() + "://" + request.serverName();
    if (!request.serverPort().empty() && request.serverPort() != "80")
      url += ":" + request.serverPort();
    url += request.path();

    // remove '/feed/'
    url.erase(url.length() - 6);
  }

  response.out() <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<rss version=\"2.0\">\n"
    "  <channel>\n"
    "    <title>" << Utils::htmlEncode(title_) << "</title>\n"
    "    <link>" << Utils::htmlEncode(url) << "</link>\n"
    "    <description>" << Utils::htmlEncode(description_) << "</description>\n";

  Dbo::Transaction t(session);

  auto const &topic = request.urlParam("topic"); // topic or all
  auto const &action = request.urlParam("action"); // last or from
  auto const &arg = request.urlParam("arg"); // timestamp or number of records

  const long limit = std::stol(arg);

  std::cout <<
    "Request URL parameters\n"
    "----------------------\n";

  auto params = request.urlParams();

  if (params.empty())
    std::cout << "(empty)\n";
  else
    for (const auto &param : params) {
      const auto &name = param.first;
      const auto &value = param.second;
      std::cout << name << ": " << value << "\n";
    }

  Journal events;
  if (action == "last" && !topic.empty())
    events = session.find<Rss>().where("topic = ?").bind(topic).orderBy("date desc").limit(limit);
  else if (action == "last" && topic.empty())
    events = session.find<Rss>().orderBy("date desc").limit(limit);
  else if (action == "from" && !topic.empty())
    events = session.find<Rss>().where("topic = ? and date >= ?").bind(topic).bind(_fromTime(limit)).orderBy("date desc");
  else if (action == "from" && topic.empty())
    events = session.find<Rss>().where("date >= ?").bind(_fromTime(limit)).orderBy("date desc");
  else
    events = session.find<Rss>().orderBy("date desc").limit(20);

  for (auto i = events.begin(); i != events.end(); ++i) {
    Dbo::ptr<Rss> rss = *i;

    std::string permaLink = url + "/" + rss->permaLink();

    response.out() <<
      "    <item>\n"
      "      <title>" << Utils::htmlEncode(rss->title.toUTF8()) << "</title>\n"
      "      <pubDate>" << rss->date.toString("ddd, d MMM yyyy hh:mm:ss UTC") << "</pubDate>\n"
      "      <guid isPermaLink=\"true\">" << Utils::htmlEncode(permaLink) << "</guid>\n";

    std::string description = rss->brief.toUTF8();
    if (!rss->body.empty())
      description += "<p><a href=\"" + permaLink + "\">Read the rest...</a></p>";

    response.out() << 
      "      <description><![CDATA[" << description << "]]></description>\n"
      "    </item>\n";
  }

  response.out() <<
    "  </channel>\n"
    "</rss>\n";

  t.commit();
}

RSSFeed::RSSFeed(Dbo::SqlConnectionPool *connectionPool, const std::string &title, const std::string &url, const std::string &description)
: connectionPool_(connectionPool)
, title_(title)
, url_(url)
, description_(description) {
}

RSSFeed::~RSSFeed() {
  beingDeleted();
}
