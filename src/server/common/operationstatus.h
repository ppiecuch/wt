#ifndef COMMON_OPERATION_STATUS
#define COMMON_OPERATION_STATUS

#include "utils.h"

#include <string>

class OperationStatus {
  bool m_status;
  err_code m_err_code;
  std::string m_exception_msg;

  public:
    bool failed() const { return m_status; }

    void set_error(const std::string& msg, err_code ec) {
      m_exception_msg = msg;
      m_err_code = ec;
    }

    err_code get_error_code() const { return m_err_code; }

    std::string get_exception_msg() const { return m_exception_msg; }

    void set_status(bool stat) { m_status = stat; }

    OperationStatus() { }
};

#endif // COMMON_OPERATION_STATUS
