#include "leaderboard.h"
#include "token.h"

#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(Token)

Token::Token() { }

Token::Token(const std::string &v, const WDateTime &e)
  : value(v)
  , expires(e)
{ }
