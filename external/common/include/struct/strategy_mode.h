#ifndef STRATEGY_MODE_H_
#define STRATEGY_MODE_H_

struct StrategyMode {
  enum Enum {
    Real,
    PlainTest,
    NextTest,
  };
  static inline const char* ToString(Enum type) {
    switch (type) {
     case Real:
      return "Real";
     case PlainTest:
      return "PlainTest";
     case NextTest:
      return "NextTest";
     default:
      return "unknown_mode";
    }
  }
};

#endif  //  STRATEGY_MODE_H_
