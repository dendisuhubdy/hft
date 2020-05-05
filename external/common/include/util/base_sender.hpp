#ifndef BASE_SENDER_HPP_
#define BASE_SENDER_HPP_

template <typename T>
class BaseSender {
 public:
  explicit BaseSender() {
  }
  virtual ~BaseSender() {
  }
  virtual void Send(const T & t) = 0;

};

#endif // BASE_SENDER_HPP_
