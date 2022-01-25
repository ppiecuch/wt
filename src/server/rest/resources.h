#ifndef RESOURCES
#define RESOURCES

#include <Wt/WResource.h>

using namespace Wt;
using namespace Wt::Http;



class Registration : public WResource {
protected:
    virtual void handleRequest(const Request &request, Response &response);

public:
    Registration() { }
    virtual ~Registration() { }
};

class Transactions : public WResource {
protected:
  virtual void handleRequest(const Request &request, Response &response);

public:
  Transactions() { }
  virtual ~Transactions() { }
};

class PortfolioList : public WResource {
protected:
  virtual void handleRequest(const Request &request, Response &response);

public:
  PortfolioList() { }
  virtual ~PortfolioList() { }
};

class Sell : public WResource {
protected:
  virtual void handleRequest(const Request &request, Response &response);

public:
  Sell() { }
  virtual ~Sell() { }
};


class Buy : public WResource {
protected:
  virtual void handleRequest(const Request &request, Response &response);

public:
  Buy() { }
  virtual ~Buy() { }
};

class Quote : public WResource {
protected:
  virtual void handleRequest(const Request &request, Response &response);

public:
  Quote() { }
  virtual ~Quote() { }
};

#endif // RESOURCES
