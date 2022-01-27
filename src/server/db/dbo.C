#include "dbo.h"

#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Dbo/backend/Sqlite3.h>

using namespace Wt;

std::unique_ptr<Dbo::SqlConnection> makeConnection(const std::string& conn) {
#ifdef DBO_POSTGRES
  auto result = new Dbo::backend::Postgres(conn);
  if (result->connection() != nullptr) {
    return std::unique_ptr<Dbo::SqlConnection>(result);
  }
#endif
  auto result = new Dbo::backend::Sqlite3(conn);
  if (result->connection() != nullptr) {
    return std::unique_ptr<Dbo::SqlConnection>(result);
  }
  throw std::runtime_error(std::string("Unable to open database: ") + conn);
}
