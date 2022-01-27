#ifndef RESOURCES
#define RESOURCES

#include <Wt/WResource.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>

using namespace Wt;

#include <string>

class ApiService  : public WResource {

  Dbo::SqlConnectionPool *connectionPool_;

  void specialcase2003();
  void yeararchive(const std::string &arg1);
  void montharchive(const std::string &arg1, const std::string &arg2);
  void articledetail(const std::string &arg1, const std::string &arg2, const std::string &arg3);

protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  ApiService(Dbo::SqlConnectionPool *connectionPool);
  virtual ~ApiService() { }
};

class Transactions : public WResource {
protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  Transactions() { }
  virtual ~Transactions() { }
};

class PortfolioList : public WResource {
protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  PortfolioList() { }
  virtual ~PortfolioList() { }
};

class Sell : public WResource {
protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  Sell() { }
  virtual ~Sell() { }
};


class Buy : public WResource {
protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  Buy() { }
  virtual ~Buy() { }
};

class Quote : public WResource {
protected:
  virtual void handleRequest(const Http::Request &request, Http::Response &response) override;

public:
  Quote() { }
  virtual ~Quote() { }
};

#endif // RESOURCES
