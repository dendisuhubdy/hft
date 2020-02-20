#include "time_controller.h"

TimeController::TimeController(string time_config)
  : timezone_diff(8*3600) {
  libconfig::Config time_cfg;
  time_cfg.readFile(time_config.c_str());
  const libconfig::Setting &sleep_time = time_cfg.lookup("time_controller")["sleep_time"];
  const libconfig::Setting &close_time = time_cfg.lookup("time_controller")["close_time"];
  const libconfig::Setting &force_close_time = time_cfg.lookup("time_controller")["force_close_time"];
  std::vector<std::string> sleep_time_v;
  std::vector<std::string> close_time_v;
  std::vector<std::string> force_close_time_v;
  for (int i = 0; i < sleep_time.getLength(); i++) {
    sleep_time_v.push_back(sleep_time[i]);
  }
  for (int i = 0; i < close_time.getLength(); i++) {
    close_time_v.push_back(close_time[i]);
  }
  for (int i = 0; i < force_close_time.getLength(); i++) {
    force_close_time_v.push_back(force_close_time[i]);
  }
  Push(sleep_time_v, sleep_start, sleep_stop);
  Push(close_time_v, close_start, close_stop);
  Push(force_close_time_v, force_close_start, force_close_stop);

  for (size_t i = 0; i < sleep_start.size(); i++) {
    printf("sleep time: %d - %d, %d:%d - %d:%d\n", sleep_start[i], sleep_stop[i], sleep_start[i]/3600, sleep_start[i]%3600/60, sleep_stop[i]/3600, sleep_stop[i]%3600/60);
  }
  for (size_t i = 0; i < close_start.size(); i++) {
    printf("close time: %d - %d, %d:%d - %d:%d\n", close_start[i], close_stop[i], close_start[i]/3600, close_start[i]%3600/60, close_stop[i]/3600, close_stop[i]%3600/60);
  }
  for (size_t i = 0; i < force_close_start.size(); i++) {
    printf("force_close time: %d - %d, %d:%d - %d:%d\n", force_close_start[i], force_close_stop[i], force_close_start[i]/3600, force_close_start[i]%3600/60, force_close_stop[i]/3600, force_close_stop[i]%3600/60);
  }
  timeval current_time;
  gettimeofday(&current_time, NULL);
  last_sec = current_time.tv_sec + timezone_diff;
  if (!Check()) {
    printf("timecontroller check failed!\n");
    exit(1);
  }

}

TimeController::TimeController() {
}

TimeController::TimeController(const TimeController & t) {
  last_sec = t.last_sec;
  timezone_diff = t.timezone_diff;
  sleep_start = t.sleep_start;
  sleep_stop = t.sleep_stop;
  close_start = t.close_start;
  close_stop = t.close_stop;
  force_close_start = t.force_close_start;
  force_close_stop = t.force_close_stop;
}

TimeController::~TimeController() {
}

std::string TimeController::IntStr(int sec) const {
  char a[32];
  time_t t(sec);
  strftime(a, sizeof(a), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
  return a;
}

int TimeController::TimevalInt(timeval t) const {
  if (t.tv_sec == 0) {
    gettimeofday(&t, NULL);
  }
  return (timezone_diff + t.tv_sec)%(24*3600);
}

std::string TimeController::TimevalStr(timeval t, bool show_time) const {
  if (t.tv_sec == 0) {
    gettimeofday(&t, NULL);
  }
  struct tm p;
  p = *localtime(&t.tv_sec);
  char buf[64];
  show_time ? strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &p) : strftime(buf, 64, "%Y-%m-%d", &p);
  return buf;
}

TimeStatus::Enum TimeController::ShotStatus(const MarketSnapshot& shot) {
    return IntStatus((shot.time.tv_sec+timezone_diff)%(24*3600));
}

TimeStatus::Enum TimeController::CurrentStatus() {
    timeval current_time;
    gettimeofday(&current_time, NULL);
    last_sec = current_time.tv_sec + timezone_diff;
    int check_time = last_sec % (24*3600);
    // printf("current time is %d, status is %s\n", check_time, TimeStatus::ToString(IntStatus(check_time)));
    return IntStatus(check_time);
}

TimeStatus::Enum TimeController::IntStatus(int check_time) const {
  for (size_t i = 0; i < sleep_start.size(); i++) {
    if (sleep_start[i] <= check_time && check_time <= sleep_stop[i]) {
      return TimeStatus::Pause;
    }
  }
  for (size_t i = 0; i < close_start.size(); i++) {
    if (close_start[i] <= check_time && check_time <= close_stop[i]) {
      // printf("close hit!%d\n", check_time);
      return TimeStatus::Close;
    }
  }
  for (size_t i = 0; i < force_close_start.size(); i++) {
    // printf("checking force: %d %d check %d\n", force_close_start[i], force_close_stop[i], check_time);
    if (force_close_start[i] <= check_time && check_time <= force_close_stop[i]) {
      // printf("forceclose hit!%d\n", check_time);
      return TimeStatus::ForceClose;
    }
  }
  return TimeStatus::Valid;
}

int TimeController::Translate(const std::string & time) const {
  std::vector<std::string> content = Split(time, ":");
  if (content.size() != 3) {
    printf("slplit time error, size is %zd, it's %s\n", content.size(), time.c_str());
    exit(1);
  }
  int hour = atoi(content[0].c_str());
  int min = atoi(content[1].c_str());
  int sec = atoi(content[2].c_str());
  return hour * 3600 + min * 60 + sec;
}

bool TimeController::IsMix(int s1, int e1, int s2, int e2) const {
  if ((s1 < s2 && s2 < e1) || (s1 < e2 && e2 < e1) || (s2 < s1 && e2 > e1)) {
    printf("s1 e1 s2 e2: %d %d %d %d\n", s1, e1, s2, e2);
    return true;
  }
  return false;
}

bool TimeController::Check() const {
  std::vector<std::pair<int, int> > time_v;
  for (size_t i = 0; i < sleep_start.size(); i++) {
    time_v.emplace_back(std::make_pair(sleep_start[i], sleep_stop[i]));
  }
  for (size_t i = 0; i < close_start.size(); i++) {
    time_v.emplace_back(std::make_pair(close_start[i], close_stop[i]));
  }
  for (size_t i = 0; i < force_close_start.size(); i++) {
    time_v.emplace_back(std::make_pair(force_close_start[i], force_close_stop[i]));
  }
  for (size_t i = 0; i < time_v.size(); i++) {
    for (size_t j = i; j < time_v.size(); j++) {
      if (IsMix(time_v[i].first, time_v[i].second, time_v[j].first, time_v[j].second)) {
        return false;
      }
    }
  }
  return true;
}

void TimeController::Push(std::vector<std::string> timestr, std::vector<int>& a, std::vector<int>& b) {
  for (size_t i = 0; i < timestr.size(); i++) {
    std::vector<std::string> tv = Split(timestr[i], "-");
    if (tv.size() != 2) {
      printf("split error!size is %zd, it's %s\n", tv.size(), timestr[i].c_str());
      exit(1);
    }
    if (Translate(tv[0]) >= Translate(tv[1])) {
      printf("time error, stoptime < starttime! it's %s\n", timestr[i].c_str());
      exit(1);
    }
    a.emplace_back(Translate(tv[0]));
    b.emplace_back(Translate(tv[1]));
  }
}

void TimeController::StartTimer() {
  timeval current_time;
  gettimeofday(&current_time, NULL);
  timer_sec = current_time.tv_sec;
  timer_usec = current_time.tv_usec;
  is_timer_on = true;
}

int TimeController::CurrentInt() const {
  timeval current_time;
  gettimeofday(&current_time, NULL);
  return (current_time.tv_sec + timezone_diff) % (24*3600);
}

void TimeController::EndTimer(const std::string & label) {
  if (!is_timer_on) {
    printf("timer is not on, cant end!\n");
    return;
  }
  timeval current_time;
  gettimeofday(&current_time, NULL);
  printf("[%s]timer running time: %ld %ld\n", label.c_str(), current_time.tv_sec - timer_sec, current_time.tv_usec-timer_usec);
  is_timer_on = false;
}
