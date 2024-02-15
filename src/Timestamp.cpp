//
// Created by xy on 2024-02-14.
//


#include "Timestamp.h"
Timestamp::Timestamp() {
  time_point_ = std::chrono::system_clock::now();
}
Timestamp::Timestamp(int64_t times) {
  time_point_ = std::chrono::system_clock::from_time_t(times);
}
Timestamp Timestamp::now() {
  return {};  // {} 可以表示一个空的初始化列表。当它用在对象初始化的上下文中时，它表示使用默认构造函数来创建一个临时对象，即返回Timestamp()
}
int64_t Timestamp::toInt() const {
  return std::chrono::system_clock::to_time_t(time_point_);
}
std::string Timestamp::toString() const {
  time_t tm = std::chrono::system_clock::to_time_t(time_point_);
  return std::ctime(&tm);
}

/*

#include <iostream>
#include <unistd.h>

int main(){

  Timestamp t1;
  std::cout << t1.toString() << std::endl;
  std::cout << t1.toInt() << std::endl;

  sleep(2);

  std::cout << Timestamp::now().toString() << std::endl;
  std::cout << Timestamp::now().toInt() << std::endl;
  return 0;
}

执行结果：

Thu Feb 15 09:46:57 2024

1707961617

Thu Feb 15 09:46:59 2024

1707961619

*/
