#ifndef DB_LEADERBOARD
#define DB_LEADERBOARD

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Auth/Dbo/AuthInfo.h>
#include "Wt/Auth/Identity.h"

#include <string>

using namespace Wt;
namespace dbo = Wt::Dbo;

class DeviceToken;
class Device;
class Team;
class Player;
class Game;
class Ranking;
class Score;
class History;

typedef Auth::Dbo::AuthInfo<Player> AuthInfo;
typedef Dbo::ptr<AuthInfo> DboAuthInfo;
typedef Dbo::collection<DboAuthInfo> DboAuthInfos;

typedef Dbo::ptr<DeviceToken> DboDeviceToken;
typedef Dbo::ptr<Device> DboDevice;
typedef Dbo::ptr<Team> DboTeam;
typedef Dbo::ptr<Player> DboPlayer;
typedef Dbo::ptr<Game> DboGame;
typedef Dbo::ptr<Ranking> DboRanking;
typedef Dbo::ptr<Score> DboScore;
typedef Dbo::ptr<History> DboHistory;

typedef Dbo::collection<Dbo::ptr<DeviceToken>> DboDeviceTokens;
typedef Dbo::collection<Dbo::ptr<Device>> DboDevices;
typedef Dbo::collection<Dbo::ptr<Team>> DboTeams;
typedef Dbo::collection<Dbo::ptr<Player>> DboPlayers;
typedef Dbo::collection<Dbo::ptr<Ranking>> DboRankings;
typedef Dbo::collection<Dbo::ptr<Score>> DboScores;


class DeviceToken : public dbo::Dbo<DeviceToken> {
public:
  DboDevice device;

  std::string value;
  WDateTime expires;

  template<class Action>
  void persist(Action &a) {
    dbo::field(a, value, "value");
    dbo::field(a, expires, "expires");

    dbo::belongsTo(a, device, "device_token"); // DeviceToken -+- Device
  }

  DeviceToken() { }
  DeviceToken(const std::string &v, const WDateTime &e) : value(v), expires(e) { }
};

class Device {
public:
  std::string deviceName;

  DboPlayer owner;
  DboScores scores;
  DboDeviceToken authToken;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, deviceName, "device_name");

    Dbo::belongsTo(a, owner, "device_owner"); // Device >- Player
    Dbo::belongsTo(a, authToken, "device_token"); // Device -+- DeviceToken
    Dbo::hasMany(a, scores, Dbo::ManyToOne, "device_scores"); // Device -< Score
  }

  Device() { }
};

class Team {
public:
  std::string teamName;
  std::string accessPin;

  DboPlayers members;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, teamName, "team_name");
    Dbo::field(a, accessPin, "access_pin");

    Dbo::hasMany(a, members, Dbo::ManyToMany, "team_memebers"); // Team >-< Player
  }

  Team() { }
};

enum class Role {
    Player = 0,
    Admin = 1,
    System = 2
};

class Player {
public:
  Role role;
  int gamesPlayed;
  long long score;
  WDateTime lastGame;
  std::string accessPin;

  DboAuthInfos authInfos;

  DboTeams teams;
  DboDevices devices;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, gamesPlayed, "games_played");
    Dbo::field(a, score, "score");
    Dbo::field(a, lastGame, "last_game_date");
    Dbo::field(a, accessPin, "access_pin");

    Dbo::hasMany(a, authInfos, Dbo::ManyToOne, "user");
    Dbo::hasMany(a, devices, Dbo::ManyToOne, "device_owner"); // Player -< Device
    Dbo::hasMany(a, teams,  Dbo::ManyToMany, "team_memebers"); // Player >-< Team
  }

  Player() : gamesPlayed(0), score(0) { }
};

class Game {
public:
  std::string gameName;
  WDateTime created;
  WDateTime modified;

  DboRankings rankings;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, gameName, "game_name");
    Dbo::field(a, created, "created");
    Dbo::field(a, modified, "modified");

    Dbo::hasMany(a, rankings, Dbo::ManyToOne,  "game"); // Game -< Ranking
  }

  Game() { }
  Game(const std::string &gameName) : gameName(gameName) { }
};

class Ranking {
public:
  std::string displayName;
  std::string rankName;

  DboGame game;
  DboScores scores;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, displayName, "display_name");
    Dbo::field(a, rankName, "rankName");

    Dbo::hasMany(a, scores, Dbo::ManyToOne,  "ranking_scores"); // Ranking -< Score
    Dbo::belongsTo(a, game, "game"); // Game -< Ranking
  }

  Ranking() { }
};

class Score {
public:
  std::string displayText;
  double scoreValue;
  WDateTime created;
  WDateTime updated;

  DboDevice device;
  DboRanking ranking;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, displayText, "display_text");
    Dbo::field(a, scoreValue, "value");
    Dbo::field(a, created, "created");
    Dbo::field(a, updated, "updated");

    Dbo::belongsTo(a, device, "device_scores"); // Score >- Device
    Dbo::belongsTo(a, ranking, "ranking_scores"); // Score >- Ranking
  }

  Score() { }
};

class History {
public:
  std::string displayText;
  double scoreValue;
  WDateTime created;

  DboDevice device;
  DboRanking ranking;

  template<class Action>
  void persist(Action &a) {
  }
};

DBO_EXTERN_TEMPLATES(DeviceToken)
DBO_EXTERN_TEMPLATES(Device)
DBO_EXTERN_TEMPLATES(Team)
DBO_EXTERN_TEMPLATES(Player)
DBO_EXTERN_TEMPLATES(Game)
DBO_EXTERN_TEMPLATES(Ranking)
DBO_EXTERN_TEMPLATES(Score)
DBO_EXTERN_TEMPLATES(History)

#endif // DB_LEADERBOARD
