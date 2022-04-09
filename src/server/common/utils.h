#ifndef COMMON_UTILS
#define COMMON_UTILS

#include <Wt/Http/Response.h>
#include <Wt/Json/Object.h>
#include <Wt/WLogger.h>

#include <time.h>
#include <string>
#include <unordered_map>
#include <iostream>

constexpr unsigned int MAX_CONTENT_LENGTH = 200;

enum err_code {
  content_length_exceeded = 0,
  device_already_exists,
  invalid_credentials,
  bad_ranking_request,
  bad_user_request,
  unknown_request,
  unknown_error,
};

template <class type_t>
type_t readJSONValue(Wt::Json::Object result, const std::string &key, type_t defaultValue) {
  try {
    return result.get(key);
  } catch (Wt::WException error) {
      std::cerr << "Attribute '" << key << "' is invalid: " << error.what() << std::endl;
      return defaultValue;
  }
}

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

void LogInfo(const std::string &msg);
void LogError(const std::string &msg);

bool readFile(const std::string &file, std::string &out_data);
std::string dateTimeString(const time_t rawTime, bool utc = true);
std::string secondsToHumanReadableTime(const time_t seconds);
std::string calculateSize(const std::size_t size);

#endif // COMMON_UTILS
