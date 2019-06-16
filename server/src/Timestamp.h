#ifndef _TIMESTAMP_H
#define _TIMESTAMP_H


#include <string>


using namespace std;

///
/// Time stamp in UTC, in microseconds resolution.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp 
{
 public:
  
  /* 构造一个微秒数为0的无效类 */
  Timestamp() : microSecondsSinceEpoch_(0) {}

  /* 构造一个微秒数为microSecondsSinceEpochArg的Timestamp */
  explicit Timestamp(int64_t microSecondsSinceEpochArg)
    : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
  {}

  /* this->microSecondsSinceEpoch_ 与 that.microSecondsSinceEpoch_互换 */
  void swap(Timestamp& that)
  {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }


  /* 将microSecondsSinceEpoch_转换成 [秒.微秒]格式的 string */
  string toString() const;

  /* 转换成   xxxx-xx-xx xx:xx:xx.xxxxxx 格式的string */
  string toFormattedString(bool showMicroseconds = true) const;

  /* 判断是否有效 */
  bool valid() const { return microSecondsSinceEpoch_ > 0; }

  /* 得到私有变量microSecondsSinceEpoch_值 */
  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
  
  /* 返回秒数 */
  time_t secondsSinceEpoch() const
  { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

  /* 得到系统当前时间并存成微秒数 返回一个新Timestamp */
  static Timestamp now();

  /* 返回一个微秒数为0的无效类 */
  static Timestamp invalid()
  {
    return Timestamp();
  }

  /* Unix时间戳经历的秒数记录为微秒数，返回Timestamp类 */
  static Timestamp fromUnixTime(time_t t)
  {
    return fromUnixTime(t, 0);
  }

  /* Unix时间戳经历的秒数及微秒数记录为微秒数，返回Timestamp类 */
  static Timestamp fromUnixTime(time_t t, int microseconds)
  {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
  }

  static const int kMicroSecondsPerSecond = 1000 * 1000; //表示每秒所对应的微秒数

 private:
  int64_t microSecondsSinceEpoch_;    //1970-01-01 00:00:00 UTC  到现在的 微秒数   
};  //end class Timestamp



inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator<=(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() <= rhs.microSecondsSinceEpoch();
}

inline bool operator>=(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() >= rhs.microSecondsSinceEpoch();
}

inline bool operator>(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
}

inline bool operator!=(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() != rhs.microSecondsSinceEpoch();
}


/* 返回两个时间差秒数 */
inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}


/* 计算 timestamp + 秒数seconds  ，返回新Timestamp类 */
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  /* 计算微秒数 */
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

#endif  // _TIMESTAMP_H
