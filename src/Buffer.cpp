//
// Created by xy on 2024-02-14.
//

#include "Buffer.h"
#include <cstring>



Buffer::Buffer(uint16_t sep)
  :sep_(sep)
{}

Buffer::~Buffer() {}

void Buffer::append(const char *data, size_t size) {
  buf_.append(data,size);
}

void Buffer::appendSep(const char *data, size_t size) {
  if (sep_ == 0){
    buf_.append(data,size);
  } else if (sep_ == 1){    // 四字节 的报头
    buf_.append((char*)&size,4);  // 记录数据的长度
    buf_.append(data,size);       // 记录数据
  }
}

size_t Buffer::size() {
  return buf_.size();
}

//首地址
const char *Buffer::data() {
  return buf_.data();
}

void Buffer::clear() {
  buf_.clear();
}

//擦除指定其实下标和长度的空间
void Buffer::erase(size_t pos, size_t len) {
  buf_.erase(pos,len);
}

// 除去报文头部，得到客户端实际传输的数据
// 实际传输的数据最后会保存在ss变量中（传递的形参）
bool Buffer::pickMessage(std::string &ss) {
  //buf_中存储这 包头+包体的数据，需要进行拆分
  if (buf_.empty()) return false;

  if (sep_ == 0){
    ss = buf_;
    buf_.clear();
  } else if (sep_ == 1){
    int len = 0;
    memcpy(&len,buf_.data(),4);   //得到 数据的长度
    if (buf_.size() < len + 4) return false;  //说明buf_中的报文内容不完整

    ss = buf_.substr(4,len);  // 把除去报文头的 实际数据存储在 ss 中
    buf_.erase(0,len + 4);
  }
  return true;
}


