
//
// Created by xy on 2024-02-14.
//

#ifndef NET__TIMESTAMP_H_
#define NET__TIMESTAMP_H_

#include <chrono>
#include <string>

class Timestamp {
 public:
  Timestamp();
  Timestamp(int64_t times);

  static Timestamp now();

  int64_t toInt() const;
  std::string toString() const;
 private:
  std::chrono::system_clock::time_point time_point_;
};

#endif //NET__TIMESTAMP_H_
