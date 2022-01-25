#ifndef COMMON_UTILS
#define COMMON_UTILS

#include <Wt/Http/Response.h>
#include <Wt/WLogger.h>

#include <string>

constexpr unsigned int MAX_CONTENT_LENGTH = 200;
constexpr unsigned int INITIAL_BALANCE = 100000;

enum err_code {
  content_length_exceeded = 0,
  bad_request_upass,
  user_already_exists,
  unknown_error,
  bad_request_stockcode,
  invalid_credentials,
  bad_request_quantity,
  bad_request_price,
  insufficient_funds_balance,
  insufficient_portfolio
};

void configLogger(Wt::WLogger& logger);
void return_bad_request(Wt::Http::Response& resp, err_code ec);
void return_bad_request_Exc(Wt::Http::Response& resp, err_code ec, const std::string& exc_msg);

const char* getSQLiteDBName();

#endif // COMMON_UTILS
