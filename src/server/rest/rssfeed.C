#include <Wt/Http/Response.h>
#include <Wt/Utils.h>

using namespace Wt;

#include "../db/rss.h"
#include "../webui/session.h"

#include "rssfeed.h"


RSSFeed::RSSFeed(
  Dbo::SqlConnectionPool *connectionPool,
  const std::string &title, const std::string &url, const std::string &description)
  : connectionPool_(connectionPool)
  , title_(title)
  , url_(url)
  , description_(description)
{ }

RSSFeed::~RSSFeed() {
  beingDeleted();
}

void RSSFeed::handleRequest(const Http::Request &request, Http::Response &response) {
  Session session(connectionPool_);

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

  Journal events = session.find<Rss>
    ("order by date desc "
     "limit ?").bind(10);

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
