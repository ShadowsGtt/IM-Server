#include "LogFile.h"
#include "FileUtil.h"

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>


LogFile::LogFile(const string& basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    mutex_(threadSafe ? new MutexLock : NULL),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0)
{
  //assert(basename.find('/') == string::npos);
  /* 生成一个新日志文件 */
  rollFile();
}

LogFile::~LogFile()
{
}


/* 将日志内容追加到文件中 */
/* logline 日志内容   len 日志长度 */
void LogFile::append(const char* logline, int len)
{
  /* 如果设置了线程安全的 */
  if (mutex_)
  {
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

/* 刷新文件缓冲到文件 */
void LogFile::flush()
{
  if (mutex_)
  {
    MutexLockGuard lock(*mutex_);
    file_->flush();
  }
  else
  {
    file_->flush();
  }
}


/* 将日志内容写到文件 并更新冲刷时间等 */
void LogFile::append_unlocked(const char* logline, int len)
{
  file_->append(logline, len);

  /* 如果写入到文件的内容大小已经超过了rollsize */
  if (file_->writtenBytes() > rollSize_)
  {
    /* 生成新文件 */
    rollFile();
  }
  else
  {
    /* 写次数+1 */
    ++count_;
    if (count_ >= checkEveryN_)
    {
      count_ = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
	/* 判断是否到了新的一天 */
      if (thisPeriod_ != startOfPeriod_)
      {
        rollFile();
      }
	/* 是否到了刷新时间 (默认3秒)  */
      else if (now - lastFlush_ > flushInterval_)
      {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}


/* 生成一个新文件  释放旧文件 */
bool LogFile::rollFile()
{
  time_t now = 0;
  /* 生成新文件名字 并将时间存入now中 */
  string filename1 = getLogFileName(basename_, &now);
  string filename = "../log/";
  filename += filename1;
  /* start为now所指的当天的00:00 */
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_)
  {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new FileUtil::AppendFile(filename));
    return true;
  }
  return false;
}

/* 生成文件名字 */
/* 文件的名字格式为 */
/* basename.年月日-十分秒.主机名.进程id.log */
string LogFile::getLogFileName(const string& basename, time_t* now)
{
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  /* struct tm *gmtime_r(const time_t *timep, struct tm *result); */
  /* 函数将日历时间timep转换为用UTC时间表示的时间 ， 将结果存入result */
  gmtime_r(now, &tm); // FIXME: localtime_r ?
  
  /* 日志文件名的时间格式  */
  /* 年月日-时分秒  */
  /* 20190213-060709 */
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  auto f = []()->std::string{  
     	char buf[256];
    	if (::gethostname(buf, sizeof buf) == 0)
  		{
    		buf[sizeof(buf)-1] = '\0';
    		return buf;
  		}
  		else
  		{
    		return "unknownhost";
  		}
  };
  filename += f();
  //filename += ProcessInfo::hostname();

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", getpid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}

