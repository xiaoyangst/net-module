//
// Created by xy on 2024-02-14.
//


#ifndef NET_INCLUDE_BUFFER_H_
#define NET_INCLUDE_BUFFER_H_

#include <string>

class Buffer{
 public:
  Buffer(uint16_t sep = 1);
  ~Buffer();

  void append(const char* data,size_t size);      // 追加数据 到buf_中
  void appendSep(const char* data,size_t size);   // 追加数据+分隔符 到buf_中
  void erase(size_t pos,size_t len);              // 从下标 pos开始 ，删除 len 字节
  size_t size();                                  // 获得buf_ 大小
  const char* data();                             // 获得buf_ 首地址
  void clear();
  bool pickMessage(std::string &ss);              //从buf_中拆分出一个报文，存放在ss中，如果buf_中没有报文，返回false
 private:
  std::string buf_;   // 存放数据
  const uint16_t sep_;  // 分隔符 我们这里采用传递报文长度解决粘包和粘包问题
};

#endif //NET_INCLUDE_BUFFER_H_
