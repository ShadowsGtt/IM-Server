#include "Date.h"
#include <stdio.h>  // snprintf

namespace detail
{

/* 年月日--- 儒略日的计算  */
int getJulianDayNumber(int year, int month, int day)
{
  int a = (14 - month) / 12;
  int y = year + 4800 - a;
  int m = month + 12 * a - 3;
  return day + (153*m + 2) / 5 + y*365 + y/4 - y/100 + y/400 - 32045;
}

/* 儒略日 --- 年月日的计算 */
struct Date::YearMonthDay getYearMonthDay(int julianDayNumber)
{
  int a = julianDayNumber + 32044;
  int b = (4 * a + 3) / 146097;
  int c = a - ((b * 146097) / 4);
  int d = (4 * c + 3) / 1461;
  int e = c - ((1461 * d) / 4);
  int m = (5 * e + 2) / 153;
  Date::YearMonthDay ymd;
  ymd.day = e - ((153 * m + 2) / 5) + 1;
  ymd.month = m + 3 - 12 * (m / 10);
  ymd.year = b * 100 + d - 4800 + (m / 10);
  return ymd;
}
}   // end namespace detail

const int Date::kJulianDayOf1970_01_01 = detail::getJulianDayNumber(1970, 1, 1);

using namespace detail;

Date::Date(int y, int m, int d)
{
  julianDayNumber_ = getJulianDayNumber(y, m, d);
}



//struct tm 
//{ 　
//	int tm_sec;		 	 /* 秒–取值区间为[0,59] */ 　　
//	int tm_min; 		 /* 分 - 取值区间为[0,59] */ 　　
//	int tm_hour; 	     /* 时 - 取值区间为[0,23] */ 　　
//	int tm_mday;	     /* 日期 - 取值区间为[1,31] */ 　
//	int tm_mon;			 /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */ 
//	int tm_year; 	     /* 年份，其值从1900开始 */ 　
//	int tm_wday; 	     /* 星期–取值区间为[0,6]，其中0代表星期天，1代表星期一*/ 　
//	int tm_yday; 	     /* 从每年的1月1日开始的天数–取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推 */ 　
//	int tm_isdst; 	     /* 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负。*/ 　
// long int tm_gmtoff;	     /*指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数*/ 　　
// const char *tm_zone;      /*当前时区的名字(与环境变量TZ有关)*/ 　
//}; 



Date::Date(const struct tm& t)
{
    julianDayNumber_ = getJulianDayNumber(t.tm_year+1900,t.tm_mon+1,t.tm_mday);
}

string Date::toIsoString() const
{
  char buf[32];
  YearMonthDay ymd(yearMonthDay());
  snprintf(buf, sizeof buf, "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
  return buf;
}


Date::YearMonthDay Date::yearMonthDay() const
{
  return getYearMonthDay(julianDayNumber_);
}

