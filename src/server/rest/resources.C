#include <Wt/WResource.h>
#include <Wt/WServer.h>

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

using namespace Wt;
using namespace Wt::Http;

#include <iostream>

#include "resources.h"
#include "../db/leaderboard.h"

void ApiService::specialcase2003() { }
void ApiService::yeararchive(const std::string &arg1) { }
void ApiService::montharchive(const std::string &arg1, const std::string &arg2) { }
void ApiService::articledetail(const std::string &arg1, const std::string &arg2, const std::string &arg3) { }

void ApiService::handleRequest(const Request &request, Response &response) {
  response.setMimeType("text/plain");

  response.out() << "Request path:\n" << request.path() << "\n\n";

  auto pathInfo = request.pathInfo();
  if (pathInfo.empty())
    pathInfo = "(empty)";
  response.out() << "Request path info:\n" << pathInfo << "\n\n";

  response.out() << "Request URL parameters\n"
                    "----------------------\n";

  auto params = request.urlParams();

  if (params.empty())
    response.out() << "(empty)\n";

  for (const auto &param : params) {
    const auto &name = param.first;
    const auto &value = param.second;
    response.out() << name << ": " << value << '\n';
  }
}

ApiService::ApiService(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) { }
