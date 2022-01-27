#include "user.h"

#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(User);

using namespace Wt::Dbo;

User::User()
  : gamesPlayed(0)
  , score(0)
{ }
