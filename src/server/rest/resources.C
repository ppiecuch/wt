#include <Wt/WResource.h>
#include <Wt/WServer.h>

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <Wt/Dbo/Session.h>

using namespace Wt;
using namespace Wt::Http;

#include <string>
#include <iostream>

#include "resources.h"
#include "../db/leaderboard.h"
#include "../common/utils.h"

static const char* error_msgs[12] = {
  "No error detected",
  "Allowable request content length exceeded",
  "Missing json content",
  "Malformed json content",
  "Bad request, missing autorizatin token",
  "Bad request, missing username/password specification",
  "Registration failed. That device name already exists",
  "Registration failed. That user name already exists",
  "",
  "",
  "Unidentified request",
  "Unknown error",
};

static std::string X_AUTH_HEADER = "Authorization";
static std::string X_AUTH_SCHEMA = "X-Wt-GameServer";

static void responseError(Response &response, err_code ec) {
  log("fatal") << Wt::WLogger::timestamp << Wt::WLogger::sep << "ERROR" << Wt::WLogger::sep << error_msgs[ec];

  response.setMimeType("application/json");

  Json::Object err;
  err["code"] = ec;
  err["details"] = error_msgs[ec];

  switch (ec) {
    case missing_autorization_token:
    case invalid_credentials:
      response.setStatus(403);
      break;
    default:
      response.setStatus(400);
  }
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

static std::string fromIstream(std::istream &stream) {
  std::istreambuf_iterator<char> eos;
  return std::string(std::istreambuf_iterator<char>(stream), eos);
}

static err_code validRequest(const Request &request, Json::Object *bodyContent = nullptr) {
  const int contentLength = request.contentLength();
  if (contentLength > MAX_CONTENT_LENGTH) {
    return content_length_exceeded;
  }
  // https://stackoverflow.com/questions/39987974/get-json-from-wthttprequest-request
  if (bodyContent) {
    if (contentLength <= 0)
      return missing_json_content;
    try {
      Json::parse(fromIstream(request.in()), *bodyContent);
    } catch(Json::ParseError &e) {
      return malformed_json_content;
    }
  }

  return no_error;
}

static err_code validSessionRequest(const Request &request, Json::Object *bodyContent = nullptr) {
  err_code err = validRequest(request, bodyContent);
  if (err == no_error) {
    // check authorization token
    std::string token = request.headerValue(X_AUTH_HEADER);
    if (token.empty() || token.rfind(X_AUTH_SCHEMA, 0) != 0)
      return missing_autorization_token;
  }
  return err;
}

/// ApiServiceRegistering

void ApiServiceRegistering::handleRequest(const Request &request, Response &response) {
  dumpRequest(request);

  Json::Object body;
  err_code err = validSessionRequest(request, &body);
  if (err != no_error) {
      responseError(response, err);
      return;
  } else {
    std::string name = body["device-name"];
    std::string player_code = body["player-code"];
    std::string team_code = body["team-code"];

    Json::Object ret;

    std::string action = request.urlParam("action");
    if (action == "register") {
    } else if (action == "rename") {
    } else if (action == "attach") {
    } else if (action == "ping") {
    } else {
      responseError(response, bad_user_request);
      return;
    }

    response.setMimeType("text/json");
    response.out() << Json::serialize(ret);
  }
}
ApiServiceRegistering::ApiServiceRegistering(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) { }
ApiServiceRegistering::~ApiServiceRegistering() { beingDeleted(); }

/// ApiServiceScore

void ApiServiceScore::handleRequest(const Request &request, Response &response) {
  dumpRequest(request);

  response.setMimeType("text/json");
}
ApiServiceScore::ApiServiceScore(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) { }
ApiServiceScore::~ApiServiceScore() { beingDeleted(); }

/// ApiServiceRanking

void ApiServiceRanking::handleRequest(const Request &request, Response &response) {
  dumpRequest(request);

  response.setMimeType("text/json");
}
ApiServiceRanking::ApiServiceRanking(Dbo::SqlConnectionPool *connectionPool) : connectionPool_(connectionPool) { }
ApiServiceRanking::~ApiServiceRanking() { beingDeleted(); }
