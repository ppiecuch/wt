#include "dbo.h"

#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Dbo/backend/Sqlite3.h>

std::unique_ptr<Wt::Dbo::SqlConnection> makeConnection(const std::string& db) {
#ifdef DBO_POSTGRES
  auto result = new Wt::Dbo::backend::Postgres(db);
  if (result->connection() != nullptr) {
    return std::unique_ptr<Wt::Dbo::SqlConnection>(result);
  }
#endif
  auto result = new Wt::Dbo::backend::Sqlite3(db);
  if (result->connection() != nullptr) {
    return std::unique_ptr<Wt::Dbo::SqlConnection>(result);
  }
  throw std::runtime_error(std::string("Unable to open database: ") + db);
}
