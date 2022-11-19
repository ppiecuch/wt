#ifndef RESOURCES
#define RESOURCES

#include <Wt/WResource.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>

using namespace Wt;

#include <string>

/// Device and users registering and authentication

class ApiServiceRegistering  : public WResource {

  Dbo::SqlConnectionPool *connectionPool_;

protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  ApiServiceRegistering(Dbo::SqlConnectionPool *connectionPool);
  virtual ~ApiServiceRegistering();
};

/// Scores management

class ApiServiceScore  : public WResource {

  Dbo::SqlConnectionPool *connectionPool_;

protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  ApiServiceScore(Dbo::SqlConnectionPool *connectionPool);
  virtual ~ApiServiceScore();
};

/// Ranking access

class ApiServiceRanking  : public WResource {

  Dbo::SqlConnectionPool *connectionPool_;

protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  ApiServiceRanking(Dbo::SqlConnectionPool *connectionPool);
  virtual ~ApiServiceRanking();
};

#endif // RESOURCES
