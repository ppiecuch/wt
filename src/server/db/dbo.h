#include <Wt/Dbo/SqlConnection.h>

#include <memory>
#include <string>

/// Returns a connected DB connection, or raises runtime_error
std::unique_ptr<Wt::Dbo::SqlConnection> makeConnection(const std::string& db);
