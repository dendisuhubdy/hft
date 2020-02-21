#ifndef OFFSET_H_
#define OFFSET_H_

struct Offset {
  enum Enum {
    UNINITED,
    OPEN,
    CLOSE_TODAY,
    CLOSE,
    NO_TODAY
  };

  static inline const char* ToString(Enum offset) {
    if (offset == Offset::OPEN) {
      return "OPEN";
    } else if (offset == Offset::CLOSE_TODAY) {
      return "CLOSE_TODAY";
    } else if (offset == Offset::CLOSE) {
      return "CLOSE";
    } else if (offset == Offset::NO_TODAY) {
      return "NO_TODAY";
    }
    return "UNKNOWN_OFFSET";
  }
};

#endif  // OFFSET_H_
