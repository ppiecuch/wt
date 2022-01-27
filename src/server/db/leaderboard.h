#ifndef DB_SCORE
#define DB_SCORE

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

namespace Wt;

class SubIndex;
class Score;

typedef Dbo::collection< Dbo::ptr<SubIndex> > SubIndexes;
typedef Dbo::collection< Dbo::ptr<Score> > Scores;

class Leaderboard {
public:
  std::string name;
  WDateTime created;
  WDateTime modified;

  SubIndexes categories;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, name, "name");
    Dbo::field(a, created, "created");
    Dbo::field(a, modified, "modified");

    Dbo::hasMany(a, categories, dbo::ManyToOne,  "leaderboard");
  }

  Leaderboard() { }
  Leaderboard(const std::string &name) : name(name) { }
};

class SubIndex {
public:
  std::string display;
  std::string value;

  Dbo::ptr<Leaderboard> leaderBoard;

  Scores scores;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, display, "display");
    Dbo::field(a, value, "value");

    Dbo::hasMany(a, scores, dbo::ManyToOne,  "score");
    Dbo::belongsTo(a, leaderBoard, "leaderboard");
  }

  SubIndex() { }
};

class Score {
public:
  std::string score_text;
  double score_value;
  WDateTime created;
  WDateTime modified;

  Dbo::ptr<User> player;
  Dbo::ptr<Leaderboard> leaderBoard;
  Dbo::ptr<SubIndex> subIndex;

  template<class Action>
  void persist(Action &a) {
    Dbo::field(a, score_text, "text");
    Dbo::field(a, score_value, "value");

    Dbo::belongsTo(a, player, "user");
    Dbo::belongsTo(a, subIndex, "subindex");
  }

  Score() { }
};

DBO_EXTERN_TEMPLATES(Leaderboard)
DBO_EXTERN_TEMPLATES(SubIndex)
DBO_EXTERN_TEMPLATES(Score)

#endif // DB_SCORE
