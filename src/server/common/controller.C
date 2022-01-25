#include "controller.h"

OperationStatus CommonController::registerNewTrader(const std::string& user, const std::string& pass) const {
  return OperationStatus();
}

int CommonController::areValidCredentials(const std::string& user, const std::string& pass, OperationStatus& op_stat) const {
  return 1;
}

int CommonController::getLastSalePrice(const std::string& stockcode, OperationStatus& op_stat) const {
  return -1;
}

std::tuple<OperationStatus, int> CommonController::getQuote(const std::string& user, const std::string& pass, const std::string& stockcode) const {
  return std::make_tuple(OperationStatus(), 1);
}

std::tuple<OperationStatus, std::vector<std::tuple<std::string, int, int, bool>>>
  CommonController::getTransactions(const std::string& user, const std::string& pass) const {

  OperationStatus op_stat;
  std::vector<std::tuple<std::string, int, int, bool > > retVec;
  return std::make_tuple(op_stat, retVec);
}

std::tuple<OperationStatus, std::vector<std::tuple<std::string, int, int, double>>>
  CommonController::getPortfolio(const std::string& user, const std::string& pass) const {

  OperationStatus op_stat;
  std::vector<std::tuple<std::string, int, int, double > > retVec;
  return std::make_tuple(op_stat, retVec);
}

std::tuple<OperationStatus, std::tuple<int, std::string, std::string>>
  CommonController::buy(const std::string& user, const std::string& pass, const std::string& stockcode, int qty, int offer_price) const {
  OperationStatus op_stat;
  std::tuple<int, std::string, std::string > retTup;
  return std::make_tuple(op_stat, retTup);
}

std::tuple<OperationStatus, std::tuple<int, std::string, std::string>>
  CommonController::sell(const std::string& user, const std::string& pass, const std::string& stockcode, int qty, int offer_price) const {
  OperationStatus op_stat;
  std::tuple<int, std::string, std::string > retTup;
  return std::make_tuple(op_stat, retTup);
}
