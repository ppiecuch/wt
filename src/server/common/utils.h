#ifndef COMMON_UTILS
#define COMMON_UTILS

#include <Wt/Http/Response.h>
#include <Wt/WLogger.h>

#include <time.h>
#include <string>
#include <unordered_map>

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

class Utility
{
public:
  template <typename _T> struct Hasher {
    std::size_t operator()(const _T &t) const {
      return std::hash<unsigned char>()(static_cast<unsigned char>(t));
    }
  };

  template <typename _T> struct HashMapper {
    typedef std::unordered_map<_T, std::string, Hasher<_T>> HashToString;
    typedef std::unordered_map<std::string, _T> HashToEnumClass;
  };
};

void return_bad_request(Wt::Http::Response& resp, err_code ec);
void return_bad_request_Exc(Wt::Http::Response& resp, err_code ec, const std::string& exc_msg);

void LogError(const char *msg);

void configLogger(Wt::WLogger& logger);
bool readFile(const std::string &file, std::string &out_data);
std::string dateTimeString(const time_t rawTime, bool utc = true);
std::string secondsToHumanReadableTime(const time_t seconds);
std::string calculateSize(const std::size_t size);
const char* getSQLiteDBName();

#endif // COMMON_UTILS
