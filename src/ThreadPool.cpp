//
// Created by xy on 2024-02-15.
//

#include "ThreadPool.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>

ThreadPool::ThreadPool(size_t threadnum, const std::string &threadtype)
    : stop_(false), threadtype_(threadtype) {

  for (size_t i = 0; i < threadnum; ++i) {
    threads_.emplace_back([this] {
      printf("create %s thread(%ld).\n", threadtype_.c_str(), syscall(SYS_gettid));     // 显示线程类型和线程ID

      while (stop_ == false) {
        std::function<void()> task;       // 用于存放出队的元素。

        {
          std::unique_lock<std::mutex> lock(this->mutex_);

          // 等待生产者的条件变量。
          this->condition_.wait(lock, [this] {
            return ((this->stop_ == true) || (this->taskqueue_.empty() == false));
          });

          // 在线程池停止之前，如果队列中还有任务，执行完再退出。
          if ((this->stop_ == true) && (this->taskqueue_.empty() == true)) return;

          // 出队一个任务。
          task = std::move(this->taskqueue_.front());
          this->taskqueue_.pop();
        }

        task();  // 执行任务。
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  stop();
}

void ThreadPool::addtask(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    taskqueue_.push(task);
  }
  condition_.notify_one();
}

void ThreadPool::stop() {
  if (stop_) return;
  stop_ = true;
  condition_.notify_all();
  for (std::thread &thread : threads_) {
    thread.join();  // 等待全部线程执行完任务后退出
  }
}

size_t ThreadPool::size() {
  return threads_.size();
}