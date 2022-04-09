#ifndef RSS_FEED
#define RSS_FEED

#include <Wt/WResource.h>

using namespace Wt;

#include <string>

class RSSFeed : public WResource {
  Dbo::SqlConnectionPool *connectionPool_;
  std::string title_, url_, description_;

protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response);

public:
  static void createRssEntry();

  RSSFeed(Dbo::SqlConnectionPool *connectionPool, const std::string &title, const std::string &url, const std::string &description);
  virtual ~RSSFeed();
};

#endif // RSS_FEED
