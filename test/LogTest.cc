#include "../src/Log.h"
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

using namespace gweb;
using namespace std;

#define LOGLINESIZE 2048

int64_t get_current_millis(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void log(int i, FILE *fp) {
  char logline[LOGLINESIZE] = {0};
  time_t t = time(nullptr);
  struct tm *ptm = localtime(&t);

  uint32_t n = snprintf(
      logline, LOGLINESIZE,
      "[%s][%04d-%02d-%02d %02d:%02d:%02d]%s:%d(%s): log test %d\n", "ERROR",
      ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour,
      ptm->tm_min, ptm->tm_sec, __FILE__, __LINE__, __FUNCTION__, i);

  uint32_t wt_len = fwrite(logline, 1, n, fp);
  if (wt_len != n) {
    std::cerr << "fwrite fail!" << std::endl;
  }
  fflush(fp);
}

//同步写入测试
void synctest() {
  printf("1 million times logtest synctest...\n");
  char logfilepath[256] = "./log_synctest";
  FILE *fp = fopen(logfilepath, "w+");
  if (fp == nullptr) {
    printf("logfile open fail!\n");
  }
  uint64_t start_ts = get_current_millis();
  for (int i = 0; i < 1000000; ++i) {
    // LOG(LoggerLevel::ERROR, "log test %d\n", i);
    log(i, fp);
  }
  uint64_t end_ts = get_current_millis();
  fclose(fp);
  printf("1 million times logtest, time use %lums, %ldw logs/second\n",
         end_ts - start_ts, 100 * 1000 / (end_ts - start_ts));
}

//单线程异步写入测试
static int logs = 100 * 10000;
void single_thread_test() {
  printf("single_thread_test...\n");
  uint64_t start_ts = get_current_millis();
  for (int i = 0; i < logs; ++i) {

    LOG_INFO("my number is number my number is my number is my number is my "
             "number is my number is my number is " +
             to_string(i));
  }
  uint64_t end_ts = get_current_millis();
  printf("1 million times logtest, time use %lums, %ldw logs/second\n",
         end_ts - start_ts, logs / (end_ts - start_ts) / 10);
}

static int threadnum = 4;
void func() {
  for (int i = 0; i < logs; ++i) {
    LOG_INFO("log test");
  }
}

void multi_thread_test() {
  printf("multi_thread_test, threadnum: %d ...\n", threadnum);
  uint64_t start_ts = get_current_millis();
  std::thread threads[threadnum];
  for (int i = 0; i < threadnum; ++i) {
    threads[i] = std::thread(&func);
  }
  for (int i = 0; i < threadnum; ++i) {
    threads[i].join();
  }
  uint64_t end_ts = get_current_millis();
  printf("%d million times logtest, time use %lums, %ldw logs/second\n",
         threadnum, end_ts - start_ts,
         threadnum * logs / (end_ts - start_ts) / 10);
}

int main(int argc, char **argv) {
  single_thread_test();
  // multi thread test
  multi_thread_test();
}
