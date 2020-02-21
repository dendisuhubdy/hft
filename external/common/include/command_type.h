#ifndef COMMAND_TYPE_H_
#define COMMAND_TYPE_H_

struct CommandType {
  enum Enum {
    Uninited,
    Pause,
    Stop,
    RealFlat,
    HedgeFlat,
    ReviseParam,
    Reboot,
    CancelAndStop
  };

  static inline const char* ToString(Enum action) {
    if (action == CommandType::Uninited)
      return "Uninited";
    if (action == CommandType::Pause)
      return "Pause";
    if (action == CommandType::Stop)
      return "Stop";
    if (action == CommandType::RealFlat)
      return "RealFlat";
    if (action == CommandType::HedgeFlat)
      return "HedgeFlat";
    if (action == CommandType::ReviseParam)
      return "ReviseParam";
    if (action == CommandType::Reboot)
      return "Reboot";
    if (action == CommandType::CancelAndStop)
      return "CancelAndStop";
    return "error_unknown_order";
  }
};

#endif  // COMMAND_TYPE_H_
