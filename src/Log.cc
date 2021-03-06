#include <cstring>
#include "Log.h"
#include "Singleton.h"
#include "Params.h"

using namespace gweb;
using namespace std;

const char *LOG_NAME = "log.txt";

Log::Log() :
    ringBuffer_(Params::BUF_LEN, string()),
    currSec_(Time::nowSec()),
    stop_(false),
    lastRead_(0),
    lastWrite_(0) {
  file_ = fopen(LOG_NAME, "w");
  // 创建后端写入线程
  backThread_ = make_unique<thread>([this] {
    this->flushToDisk();
  });
}

Log::~Log() {
  stop_.store(true);
  condVar_.notify_one();
  backThread_->join();
  fclose(file_);
}

void Log::write(Level level, int val) {
  write(levelString[level] + to_string(val));
}

void Log::write(Level level, unsigned int val) {
  write(levelString[level] + to_string(val));
}

void Log::write(Level level, const char *log) {
  write(levelString[level] + log);
}

void Log::write(Level level, char c) {
  write(levelString[level] + c);
}

void Log::write(Level level, const string &log) {
  write(levelString[level] + log);
}

void Log::write(const string &log) {
  if (Time::nowSec() > currSec_) {
    // 更新缓存时间
    updateTime();
  }
  // 构造写入的日志行
  string writeLog(log.size() + timeStr_.size() + 2, ' '); // 实际写入的日志大小
  memcpy(writeLog.data(), timeStr_.data(), timeStr_.size());
  writeLog[timeStr_.size()] = ' ';
  memcpy(writeLog.data() + (timeStr_.size() + 1), log.data(), log.size());
  writeLog.back() = '\n';
  // 找到写入的位置
  auto currWrite = lastWrite_.fetch_add(1);
  ringBuffer_[currWrite & (Params::BUF_LEN - 1)] = std::move(writeLog);
  if (currWrite == lastRead_.load()) {
    // 唤醒后端刷新线程
    condVar_.notify_one();
  }
}

void Log::updateTime() {
  currSec_ = Time::now();
  struct tm *tm;

}

void Log::flushToDisk() {
  while (true) {
    if (stop_.load() && lastWrite_.load() == lastRead_.load()) {
      return;
    }
    while (lastRead_.load() < lastWrite_.load()) {
      // 还有日志没有写完
      while (lastRead_.load() < lastWrite_.load()) {
        int64_t currRead = lastRead_.fetch_add(1);
        string data = ringBuffer_[currRead & (Params::BUF_LEN - 1)];
        fwrite(data.data(), data.size(), 1, file_);

        data.clear();
      }
      // 写完了所有的日志，刷新缓冲区
      fflush(file_);
      // 所有的日志都写完了，写线程等待被唤醒
      {
        unique_lock<mutex> lock(mutex_);
        while (!stop_.load() && lastRead_.load() == lastWrite_.load()) {
          condVar_.wait(lock);
        }
      }
    }
  }
}

