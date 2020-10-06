#ifndef WEBSERVER_SRC_BASE_NONCOPYABLE_H
#define WEBSERVER_SRC_BASE_NONCOPYABLE_H

namespace gweb {
// 不可复制的类
class noncopyable {
public:
  noncopyable(){};
  ~noncopyable(){};

  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(const noncopyable &) = delete;
};
} // namespace gweb

#endif // WEBSERVER_SRC_BASE_NONCOPYABLE_H