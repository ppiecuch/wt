#ifndef DB_LEADERBOARD
#define DB_LEADERBOARD

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

#include <string>

using namespace Wt;

class User;
class Game;
class Ranking;
class Score;

typedef Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Dbo::ptr<AuthInfo> DboAuthInfo;
typedef Dbo::collection<DboAuthInfo> DboAuthInfos;

typedef Dbo::ptr<User> DboUser;
typedef Dbo::ptr<Game> DboGame;
typedef Dbo::ptr<Ranking> DboRanking;
typedef Dbo::ptr<Score> DboScore;

typedef Dbo::collection<Dbo::ptr<User>> DboUsers;
typedef Dbo::collection<Dbo::ptr<Ranking>> DboRankings;
typedef Dbo::collection<Dbo::ptr<Score>> DboScores;


class User {
public:
  std::string name; // a copy of auth info's user name
  int gamesPlayed;
  long long score;
  WDateTime lastGame;

  DboAuthInfos authInfos;
  DboScores scores;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, gamesPlayed, "gamesPlayed");
    Dbo::field(a, score, "score");
    Dbo::field(a, lastGame, "lastGame");

    Dbo::hasMany(a, authInfos, Dbo::ManyToOne, "user");
    Dbo::hasMany(a, scores, Dbo::ManyToOne,  "player");
  }

  User() : gamesPlayed(0), score(0) { }
};

class Game {
public:
  std::string name;
  WDateTime created;
  WDateTime modified;

  DboRankings rankings;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, name, "name");
    Dbo::field(a, created, "created");
    Dbo::field(a, modified, "modified");

    Dbo::hasMany(a, rankings, Dbo::ManyToOne,  "game");  // Game <> Ranking
  }

  Game() { }
  Game(const std::string &name) : name(name) { }
};

class Ranking {
public:
  std::string display;
  std::string name;

  DboGame game;
  DboScores scores;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, display, "display");
    Dbo::field(a, name, "name");

    Dbo::hasMany(a, scores, Dbo::ManyToOne,  "ranking");// Ranking <> Score
    Dbo::belongsTo(a, game, "game"); // Game <> Ranking
  }

  Ranking() { }
};

class Score {
public:
  std::string scoreText;
  double scoreValue;
  WDateTime created;
  WDateTime modified;

  DboUser player;
  DboRanking ranking;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, scoreText, "text");
    Dbo::field(a, scoreValue, "value");

    Dbo::belongsTo(a, player, "player"); // User <> Score
    Dbo::belongsTo(a, ranking, "ranking"); // Ranking <> Score
  }

  Score() { }
};

DBO_EXTERN_TEMPLATES(Game)
DBO_EXTERN_TEMPLATES(Ranking)
DBO_EXTERN_TEMPLATES(Score)

#endif // DB_LEADERBOARD
