#ifndef BASE_RECVER_HPP_
#define BASE_RECVER_HPP_

template <typename T>
class BaseRecver {
 public:
  BaseRecver() {
  }

  virtual ~BaseRecver() {
  }

  virtual void Recv(T& t) = 0;
};

#endif  //  BASE_RECVER_HPP_
