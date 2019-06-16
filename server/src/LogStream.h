#ifndef _LOGSTREAM_H
#define _LOGSTREAM_H


#include "Types.h"
#include "nocopyable.h"

#include <assert.h>
#include <cstring> // memcpy
#include <string>



/* FixedBuffer  是固定大小的缓冲区 用于作为日志系统的缓冲区 */
namespace detail
{

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000*1000;

template<int SIZE>
class FixedBuffer : noncopyable
{
 public:
  FixedBuffer() : cur_(data_)
  {
    setCookie(cookieStart);
  }

  ~FixedBuffer()
  {
    setCookie(cookieEnd);
  }

  /* 将长度为len的字符串添加至缓冲区 */
  void append(const char* /*restrict*/ buf, size_t len)
  {
    // FIXME: append partially
    if (implicit_cast<size_t>(avail()) > len)
    {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  /* 返回缓冲区的起始位置 */
  const char* data() const { return data_; }
  
  /* 缓冲区已有数据长度 */
  int length() const { return static_cast<int>(cur_ - data_); }

  /* 缓冲区可写位置 */
  char* current() { return cur_; }
  
  /* 缓冲区可写长度 */
  int avail() const { return static_cast<int>(end() - cur_); }
  
  /* 将cur指针向后移动len */   /*  cur一直指向可写位置 */
  void add(size_t len) { cur_ += len; }

  /* 重置cur至缓冲区头部 */
  void reset() { cur_ = data_; }
  
  /* 清空缓冲区 */
  void bzero() { ::bzero(data_, sizeof data_); }

  // for used by GDB
  const char* debugString();

  /* 设置cookie函数 用于coredump中找到丢失的日志 */
  void setCookie(void (*cookie)()) { cookie_ = cookie; }
  
  /* 这两个函数用于调试缓冲区 */
  /* 以string形式返回缓冲区的内容 */
  string toString() const { return string(data_, length()); }

 private:
  /* 返回缓冲区尾部位置 */
  const char* end() const { return data_ + sizeof data_; }


  static void cookieStart();
  static void cookieEnd();

  void (*cookie_)();
  char data_[SIZE];
  char* cur_;
};   // end class FixedBuffer

}   // end namespace detail




/*****************************************************************/

/* LogStream类是重载  >> 的日志流 */

class LogStream : noncopyable
{
  typedef LogStream self;
 public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

  /*  用法 : LogStream << true; */
  /* 会将结果追加到缓冲区Buffer中 */
  self& operator<<(bool v)
  {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }
  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float v)
  {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  self& operator<<(char v)
  {
    buffer_.append(&v, 1);
    return *this;
  }

  self& operator<<(const char* str)
  {
    if (str)
    {
      buffer_.append(str, strlen(str));
    }
    else
    {
      buffer_.append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(const unsigned char* str)
  {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const string& v)
  {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

  self& operator<<(const Buffer& v)
  {
    //*this << v.toStringPiece();
   *this << v.toString();
    return *this;
  }

  /* 将指定长度的内容追加到缓冲区Buffer */
  void append(const char* data, int len) { buffer_.append(data, len); }
  
  /* 返回缓冲区Buffer的引用 */
  const Buffer& buffer() const { return buffer_; }
  
  /* 重置缓冲区  即将cur指针指向缓冲区起始位置 */
  void resetBuffer() { buffer_.reset(); }

 private:
  void staticCheck();

  /* 把整形数转换成字符串并写入缓冲区 */
  template<typename T>
  void formatInteger(T);

  Buffer buffer_;

  static const int kMaxNumericSize = 32;
};   //end class LogStream


/******************************************************************/


class Fmt 
{
 public:
  template<typename T>
  Fmt(const char* fmt, T val);

  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};   //end class Fmt

/******************************************************************/



/* Usage:  LogStream << Fmt ; */
inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
  s.append(fmt.data(), fmt.length());
  return s;
}


#endif  // _LOGSTREAM_H

