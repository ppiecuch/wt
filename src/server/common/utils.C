#include "utils.h"

#include <boost/format.hpp>

#include <fstream>
#include <vector>

using namespace Wt;

static void log(const std::string &entry_type, const std::string &level, const std::string &message){
  WLogger wl;
  configLogger(wl);

  WLogEntry entry = wl.entry(entry_type);
  entry << WLogger::timestamp << WLogger::sep << level << WLogger::sep << message;
}


const char* SQLITE_DB_NAME = "server.db";

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

void configLogger(WLogger& logger) {
 logger.addField("datetime", false);
 logger.addField("type", false);
 logger.addField("message", true);
 
 logger.setFile("server.log");
}

void LogError(const char *msg) { log("fatal", "ERROR", msg); }

void return_bad_request(Http::Response& resp, err_code ec){
  WLogger wl;
  configLogger(wl);

  WLogEntry entry = wl.entry("fatal");
  entry << WLogger::timestamp << WLogger::sep << "ERROR" << WLogger::sep << error_msgs[ec];

  resp.setMimeType("application/json");
  std::ostream& out = resp.out();
  out << "{'status': 'Failure', 'reason': '" << error_msgs[ec] << "'}" << std::endl;
}

void return_bad_request_Exc(Http::Response& resp, err_code ec, const std::string& excep_msg) {
  WLogger wl;
  configLogger(wl);

  WLogEntry entry = wl.entry("fatal");
  entry << WLogger::timestamp << WLogger::sep << "ERROR" << WLogger::sep << error_msgs[ec] + std::string(":") + excep_msg;

  resp.setMimeType("application/json");
  std::ostream& out = resp.out();
  out << "{'status': 'FAILURE', 'reason': '" << error_msgs[ec] << "'}" << std::endl;
}

bool readFile(const std::string &file, std::string &out_data) {
  try {
    out_data.clear();
    std::ifstream ifs(file);
    out_data.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    return true;
  } catch (const std::ifstream::failure &ex) {
    LogError(ex.what());
  } catch (const std::exception &ex) {
    LogError(ex.what());
  } catch (...) {
    LogError("Unknown error!");
  }

  return false;
}

std::string dateTimeString(const time_t rawTime, bool utc) {
  if (utc) {
    return asctime(gmtime(&rawTime));
  } else {
    return asctime(localtime(&rawTime));
  }
}

std::string secondsToHumanReadableTime(const time_t seconds) {
  tm *timeInfo = gmtime(&seconds);
  std::stringstream ss;
  int years = timeInfo->tm_year - 70; // years since 1900

  if (years > 0) {
    ss << years;
    if (years == 1) {
        ss << " year";
    } else {
        ss << " years";
    }
  }

  int months = timeInfo->tm_mon; // months since January, 0-11

  if (months > 0) {
    if (ss.str().size() > 0) {
        ss << ", ";
    }

    ss << months;
    if (months == 1) {
        ss << " month";
    } else {
        ss << " months";
    }
  }

  int days = timeInfo->tm_mday - 1; // day of the month, 1-31

  if (days > 0) {
    if (ss.str().size() > 0) {
        ss << ", ";
    }

    ss << days;
    if (days == 1) {
        ss << " day";
    } else {
        ss << " days";
    }
  }

  int hours = timeInfo->tm_hour; // hours since midnight, 0-23

  if (hours > 0) {
    if (ss.str().size() > 0) {
        ss << ", ";
    }

    ss << hours;
    if (hours == 1) {
        ss << " hour";
    } else {
        ss << " hours";
    }
  }

  int minutes = timeInfo->tm_min; // minutes after the hour, 0-59

  if (minutes > 0) {
    if (ss.str().size() > 0) {
        ss << ", ";
    }

    ss << minutes;
    if (minutes == 1) {
        ss << " minute";
    } else {
        ss << " minutes";
    }
  }

  int seconds_ = timeInfo->tm_sec; // seconds after the minute, 0-61

  if (seconds_ > 0) {
    if (ss.str().size() > 0) {
        ss << ", ";
    }

    ss << seconds_;
    if (seconds_ == 1) {
        ss << " second";
    } else {
        ss << " seconds";
    }
  }

  return ss.str();
}

std::string calculateSize(const std::size_t size) {
  static const std::vector<std::string> units { "EB", "PB", "TB", "GB", "MB", "KB", "B" };
  static const size_t exbibytes = 1024ull * 1024ull * 1024ull * 1024ull * 1024ull * 1024ull;

  size_t multiplier = exbibytes;
  std::string result;

  if (size != 0) {
    for (size_t i = 0; i < units.size(); i++, multiplier /= 1024) {
      if (size < multiplier)
        continue;

      if (size % multiplier == 0) {
        result.assign((boost::format("%s %s") % (size / multiplier) % units[i]).str());
      } else {
        result.assign((boost::format("%.2f %s") % (static_cast<float>(size) / static_cast<float>(multiplier)) % units[i]).str());
      }

      return result;
    }
  }

  result.assign("0");
  return result;
}
