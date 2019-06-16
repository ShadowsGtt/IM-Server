#ifndef _LOGFILE_H
#define _LOGFILE_H

#include <memory>


#include "Mutex.h"
#include "Types.h"
#include "nocopyable.h"



namespace FileUtil
{
class AppendFile;
}



class LogFile : noncopyable
{
 public:
  LogFile(const string& basename,
          off_t rollSize,
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 1024);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
  bool rollFile();

 private:
  void append_unlocked(const char* logline, int len);

  /* 日志文件的名字 */
  static string getLogFileName(const string& basename, time_t* now);

  /* 文件名字  不带路径 */
  const string basename_;

  /* 日志文件到rollSize大小就会刷新到磁盘 */
  const off_t rollSize_;

  /* 每隔flushInterval_秒就将日志内容刷到磁盘 */
  const int flushInterval_;

  /* 每checkEveryN_次写就判断是否需要回滚 */
  const int checkEveryN_;

  /* 写入的次数 */
  int count_;

  /* 锁 */
  std::unique_ptr<MutexLock> mutex_;


  /* 开始记录日志时间  当天的任何时间都会被调整为00:00 */
  /* 例如20191002 08:10 会被调整为 20191002 00:00 */
  time_t startOfPeriod_;

  /* 最后一次回滚时间 */
  time_t lastRoll_;
  
  /* 最后一次刷新时间 */
  time_t lastFlush_;
  
  std::unique_ptr<FileUtil::AppendFile> file_;

  /* 新创建日志文件的时间间隔  24小时 */
  const static int kRollPerSeconds_ = 60*60*24;

}; // end class LogFile


#endif  // _LOGFILE_H
