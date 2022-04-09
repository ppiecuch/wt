#include <Wt/WResource.h>
#include <Wt/WServer.h>

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <Wt/Dbo/Session.h>

using namespace Wt;
using namespace Wt::Http;

#include <iostream>

#include "resources.h"
#include "../db/leaderboard.h"
#include "../common/utils.h"

static const char* error_msgs[10] = {
  "Allowable request content length exceeded",
  "Bad request, missing username/password specification",
  "Registration failed. User name already exists",
  "Unknown exception/error in persistence access",
  "Bad request, missing/invalid stock code for quote",
  "Bad request, Invalid credentials passed",
  "Bad request, in specifying quantity",
  "Bad request, in specifying price",
  "Bad request, balance cash not sufficient for the buy transaction",
  "Bad request, portfolio does not have sufficient quantity of stock"
};


static void responseError(Response &response, err_code ec) {
  log("fatal") << Wt::WLogger::timestamp << Wt::WLogger::sep << "ERROR" << Wt::WLogger::sep << error_msgs[ec];

  response.setMimeType("application/json");

  Json::Object err;
  err["status"] = "FAILURE";
  err["code"] = ec;
  err["reason"] = error_msgs[ec];

  response.out() << Json::serialize(err);
}

static void dumpRequest(const Request &request) {
  std::cout << "Request path: " << request.path() << "(" << request.method() << ")" << "\n";

  auto pathInfo = request.pathInfo();
  if (pathInfo.empty())
    pathInfo = "(empty)";

  std::cout << "Request path info: " << pathInfo << "\n";
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
}

// Resources hamdlers: Users

void ApiServiceRegistering::handleRequest(const Request &request, Response &response) {
  dumpRequest(request);

  response.setMimeType("text/plain");
}
ApiServiceRegistering::ApiServiceRegistering(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) { }
ApiServiceRegistering::~ApiServiceRegistering() { beingDeleted(); }

// Scoring

void ApiServiceScore::handleRequest(const Request &request, Response &response) {
  dumpRequest(request);

  response.setMimeType("text/plain");
}
ApiServiceScore::ApiServiceScore(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) { }
ApiServiceScore::~ApiServiceScore() { beingDeleted(); }

// Rankings

void ApiServiceRanking::handleRequest(const Request &request, Response &response) {
  dumpRequest(request);

  response.setMimeType("text/plain");
}
ApiServiceRanking::ApiServiceRanking(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) { }
ApiServiceRanking::~ApiServiceRanking() { beingDeleted(); }
