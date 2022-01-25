#include "utils.h"

const char* SQLITE_DB_NAME = "Market.db";

const char* getSQLiteDBName() {
  return SQLITE_DB_NAME;
}

const char* error_msgs[10] = {
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

void configLogger(Wt::WLogger& logger) {
 logger.addField("datetime", false);
 logger.addField("type", false);
 logger.addField("message", true);
 
 logger.setFile("server.log");
}

void return_bad_request(Wt::Http::Response& resp, err_code ec){
  Wt::WLogger wl;
  configLogger(wl);

  Wt::WLogEntry entry = wl.entry("fatal");
  entry << Wt::WLogger::timestamp << Wt::WLogger::sep << "ERROR" << Wt::WLogger::sep << error_msgs[ec];

  resp.setMimeType("application/json");
  std::ostream& out = resp.out();
  out << "{'status': 'Failure', 'reason': '" << error_msgs[ec] << "'}" << std::endl;
}

void return_bad_request_Exc(Wt::Http::Response& resp, err_code ec, const std::string& excep_msg) {
  Wt::WLogger wl;
  configLogger(wl);

  Wt::WLogEntry entry = wl.entry("fatal");
  entry << Wt::WLogger::timestamp << Wt::WLogger::sep << "ERROR" << Wt::WLogger::sep << error_msgs[ec] + std::string(":") + excep_msg;

  resp.setMimeType("application/json");
  std::ostream& out = resp.out();
  out << "{'status': 'FAILURE', 'reason': '" << error_msgs[ec] << "'}" << std::endl;
}
