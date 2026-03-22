/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Time.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <sys/time.h>
#include <math.h>
#include <string>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/rtc.h>

#include "CLApp_String.h"
#include "CLApp_Time.h"


const char *TimeZoneIndex[] = {
	"GMT+1200", // "Pacific/Kwajalein"},	// 0
	"GMT+1100", // "Pacific/Midway"}, 	// 1
	"GMT+1000", // "US/Hawaii"},			// 2
	"GMT+0900", // "US/Alaska"},			// 3
	"GMT+0800", // "US/Pacific"}, 		// 4
	"GMT+0700", // "US/Mountain"},		// 5
	"GMT+0600", // "US/Central"}, 		// 6
	"GMT+0500", // "US/Eastern"}, 		// 7
	"GMT+0400", // "Canada/Atlantic"},	// 8	
	"GMT+0300", // "America/Buenos_Aires"},	// 9
	"GMT+0200", // "GMT+0200"},				// 10
	"GMT+0100", // "Atlantic/Azores"},		// 11
	"GMT+0000",// "Europe/London"},				// 12
	"GMT-0100", // "Europe/Berlin"},			// 13
	"GMT-0200", // "Europe/Athens"},			// 14
	"GMT-0300", // "Europe/Moscow"},			// 15
	"GMT-0400", // "Asia/Muscat"},			// 16
	"GMT-0500", // "Asia/Karachi"},			// 17
	"GMT-0600", // "Asia/Dhaka"}, 			// 18
	"GMT-0700", // "Asia/Bangkok"},			// 19
	"GMT-0800", // "Asia/Taipei"},			// 20
	"GMT-0900", // "Asia/Tokyo"}, 			// 21
	"GMT-1000", // "Australia/Brisbane"}, 	// 22
	"GMT-1100", // "Asia/Magadan"},			// 23
	"GMT-1200", // "Pacific/Fiji"},			// 24
	"GMT-1300", // "Nukualofa"},				// 25
};


CLAppDateTime::CLAppDateTime()
{
	mCurrentTime.tv_sec = 0;
	mCurrentTime.tv_usec = 0;
}

CLAppDateTime::CLAppDateTime(const CLAppDateTime& t)
{
	CLAppDateTime *pt = (CLAppDateTime*)&t;
	pt->getTime(&mCurrentTime);
}

CLAppDateTime::CLAppDateTime(const struct timeval& t)
{
	memcpy(&mCurrentTime, &t, sizeof(struct timeval));
}

CLAppDateTime::CLAppDateTime(const long tv_sec, const long tv_usec)
{
	mCurrentTime.tv_sec = tv_sec;
	mCurrentTime.tv_usec = tv_usec;
}

CLAppDateTime::CLAppDateTime(const time_t t)
{
	mCurrentTime.tv_sec = t;
	mCurrentTime.tv_usec = 0;
}

CLAppDateTime::CLAppDateTime(const struct tm& t)
{
	struct tm* pt = (struct tm*)&t;
	mCurrentTime.tv_sec = mktime(pt);
	mCurrentTime.tv_usec = 0;
}

CLAppDateTime::CLAppDateTime(const int nYear, const int nMonth, const int nDay)
{
	setTime(nYear, nMonth, nDay, 0, 0, 0, 0, 0);
}

CLAppDateTime::CLAppDateTime(const int nYear, const int nMonth, const int nDay, 
			const int nHour, const int nMinute, const int nSecond)
{
	setTime(nYear, nMonth, nDay, nHour, nMinute, nSecond, 0, 0);
}

CLAppDateTime::CLAppDateTime(const int nYear, const int nMonth, const int nDay,
			const int nHour, const int nMinute, const int nSecond, 
			const int nMilisecond)
{
	setTime(nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilisecond, 0);
}

CLAppDateTime::CLAppDateTime(const int nYear, const int nMonth, const int nDay,
			const int nHour, const int nMinute, const int nSecond, 
			const int nMilisecond, const int nMicrosecond)
{
	setTime(nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilisecond, nMicrosecond);
}

CLAppDateTime::~CLAppDateTime()
{
}

void CLAppDateTime::setUserTime(const int nYear, const int nMonth, const int nDay,
						 const int nHour, const int nMinute, const int nSecond, 
						 const int nMilisecond, const int nMicrosecond)
{
	int nMonthDays = 31;
	struct tm t ; // = {0, 0, 0, 0, 0, 0, 0, 0, 0 };

	if(nYear < 1970)		t.tm_year = 0;
	else if(nYear > 2037)	t.tm_year = 2037 - 1900;
	else					t.tm_year = nYear - 1900;

	if(nMonth < 1)			t.tm_mon = 0;
	else if(nMonth > 12)	t.tm_mon = 11;
	else					t.tm_mon = nMonth - 1;

	nMonthDays = getMonthDays(t.tm_year + 1900, t.tm_mon + 1);
	
	if(nDay < 1)					t.tm_mday = nMonthDays;
	else if(nDay > nMonthDays)		t.tm_mday = 1;
	else							t.tm_mday = nDay;

	if(nHour < 0)			t.tm_hour = 0;
	else if(nHour > 23)		t.tm_hour = 23;
	else					t.tm_hour = nHour;

	if(nMinute < 0)			t.tm_min = 0;
	else if(nMinute > 59)	t.tm_min = 59;
	else					t.tm_min = nMinute;

	if(nSecond < 0)			t.tm_sec = 0;
	else if(nSecond > 59)	t.tm_sec = 59;
	else					t.tm_sec = nSecond;

	mCurrentTime.tv_sec = mktime(&t);

	mCurrentTime.tv_usec = 0;

	if(nMilisecond < 0)		mCurrentTime.tv_usec += 0;
	else if(nMilisecond > 999)	mCurrentTime.tv_usec += 999000;
	else					mCurrentTime.tv_usec += (nMilisecond * 1000);

	if(nMicrosecond < 0)	mCurrentTime.tv_usec += 0;
	else if(nMicrosecond > 999) mCurrentTime.tv_usec += 999;
	else					mCurrentTime.tv_usec += nMicrosecond;
}

void CLAppDateTime::setTime(const int nYear, const int nMonth, const int nDay,
						 const int nHour, const int nMinute, const int nSecond, 
						 const int nMilisecond, const int nMicrosecond)
{
	int nMonthDays = 31;
	struct tm t; // = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	if(nYear < 1970)		t.tm_year = 0;
	else if(nYear > 2037)	t.tm_year = 2037 - 1900;
	else					t.tm_year = nYear - 1900;

	if(nMonth < 1)			t.tm_mon = 0;
	else if(nMonth > 12)	t.tm_mon = 11;
	else					t.tm_mon = nMonth - 1;

	nMonthDays = getMonthDays(t.tm_year + 1900, t.tm_mon + 1);

	if(nDay < 1)					t.tm_mday = 1;
	else if(nDay > nMonthDays)		t.tm_mday = nMonthDays;
	else							t.tm_mday = nDay;

	if(nHour < 0)			t.tm_hour = 0;
	else if(nHour > 23)		t.tm_hour = 23;
	else					t.tm_hour = nHour;

	if(nMinute < 0)			t.tm_min = 0;
	else if(nMinute > 59)	t.tm_min = 59;
	else					t.tm_min = nMinute;

	if(nSecond < 0)			t.tm_sec = 0;
	else if(nSecond > 59)	t.tm_sec = 59;
	else					t.tm_sec = nSecond;

	mCurrentTime.tv_sec = mktime(&t);

	mCurrentTime.tv_usec = 0;

	if(nMilisecond < 0)		mCurrentTime.tv_usec += 0;
	else if(nMilisecond > 999)	mCurrentTime.tv_usec += 999000;
	else					mCurrentTime.tv_usec += (nMilisecond * 1000);

	if(nMicrosecond < 0)	mCurrentTime.tv_usec += 0;
	else if(nMicrosecond > 999) mCurrentTime.tv_usec += 999;
	else					mCurrentTime.tv_usec += nMicrosecond;
}


void CLAppDateTime::setTime(struct timeval* pt)
{
	if(pt != NULL)
	{
		memcpy(&mCurrentTime, pt, sizeof(struct timeval));
	}
}


void CLAppDateTime::setTime(struct tm* pt)
{
	if(pt != NULL)
	{
		struct timeval tv;
		tv.tv_sec = mktime(pt);
		tv.tv_usec = 0;

		setTime(&tv);
	}
}


void CLAppDateTime::setTime(time_t t)
{
	struct timeval tv = { t, 0 };
	setTime(&tv);
}


struct timeval *CLAppDateTime::getTime(struct timeval* pt)
{
	if(pt != NULL)
	{
		//*pt = mCurrentTime;
		memcpy(pt, &mCurrentTime, sizeof(struct timeval));
	}

	return pt;
}


struct tm *CLAppDateTime::getTime(struct tm* pt)
{
	if(pt != NULL)
	{
		struct timeval tv;
		getTime(&tv);
		gmtime_r(&tv.tv_sec, pt);
	}

	return pt;
}


time_t CLAppDateTime::getTime()
{
	struct timeval tv;
	getTime(&tv);
	return tv.tv_sec;
}


CLAppDateTime CLAppDateTime::getFirstTimeOfMonth()
{
	struct tm ti;
	gmtime_r(&mCurrentTime.tv_sec, &ti);
	ti.tm_mday = 1;
	ti.tm_hour = 0;
	ti.tm_min = 0;
	ti.tm_sec = 0;
	return CLAppDateTime(ti);
}


CLAppDateTime CLAppDateTime::getLastTimeOfMonth()
{
	struct tm ti;
	gmtime_r(&mCurrentTime.tv_sec, &ti);
	ti.tm_mday = 27;
	ti.tm_hour = 0;
	ti.tm_min = 0;
	ti.tm_sec = 0;


	time_t ct = mktime(&ti);
	struct tm tc;

	do
	{
		ct += 86400L;
		gmtime_r(&ct, &tc);
	} while(ti.tm_mon == tc.tm_mon);

	return CLAppDateTime(--ct);
}


CLAppDateTime CLAppDateTime::getFirstTimeOfYear()
{
	struct tm ti;
	gmtime_r(&mCurrentTime.tv_sec, &ti);
	ti.tm_mon = 0;
	ti.tm_mday = 1;
	ti.tm_hour = 0;
	ti.tm_min = 0;
	ti.tm_sec = 0;
	return CLAppDateTime(ti);
}


CLAppDateTime CLAppDateTime::getLastTimeOfYear()
{
	struct tm ti;
	gmtime_r(&mCurrentTime.tv_sec, &ti);
	ti.tm_mon = 11;
	ti.tm_mday = 31;
	ti.tm_hour = 23;
	ti.tm_min = 59;
	ti.tm_sec = 59;
	return CLAppDateTime(ti);
}


CLAppDateTime CLAppDateTime::getFirstTimeOfDay()
{
	struct tm ti;
	gmtime_r(&mCurrentTime.tv_sec, &ti);
	ti.tm_hour = 0;
	ti.tm_min = 0;
	ti.tm_sec = 0;
	return CLAppDateTime(ti);
}


CLAppDateTime CLAppDateTime::getLastTimeOfDay()
{
	struct tm ti;
	gmtime_r(&mCurrentTime.tv_sec, &ti);
	ti.tm_hour = 23;
	ti.tm_min = 59;
	ti.tm_sec = 59;
	return CLAppDateTime(ti);
}


int CLAppDateTime::getYear()
{
	struct tm t;
	gmtime_r(&mCurrentTime.tv_sec, &t);

	return t.tm_year + 1900;
}


int CLAppDateTime::getMonth()
{
	struct tm t;
	gmtime_r(&mCurrentTime.tv_sec, &t);

	return t.tm_mon;
}


int CLAppDateTime::getDay()
{
	struct tm t;
	gmtime_r(&mCurrentTime.tv_sec, &t);

	return t.tm_mday;
}


int CLAppDateTime::getHour()
{
	struct tm t;
	gmtime_r(&mCurrentTime.tv_sec, &t);

	return t.tm_hour;
}


int CLAppDateTime::getMinute()
{
	struct tm t;
	gmtime_r(&mCurrentTime.tv_sec, &t);

	return t.tm_min;
}


int CLAppDateTime::getSecond()
{
	struct tm t;
	gmtime_r(&mCurrentTime.tv_sec, &t);
	return t.tm_sec;
}


int CLAppDateTime::getDayOfWeek()
{
	struct tm t;
	gmtime_r(&mCurrentTime.tv_sec, &t);
	return t.tm_wday;
}


int CLAppDateTime::getMilisecond()
{
	return (mCurrentTime.tv_usec / 1000);
}


int CLAppDateTime::getMicrosecond()
{
	return (mCurrentTime.tv_usec % 1000);
}


void CLAppDateTime::addYear(int nYear)
{
	mCurrentTime.tv_sec += (nYear * 31536000L);
}


void CLAppDateTime::addMonth(int nMonth)
{
	mCurrentTime.tv_sec += (nMonth * 2592000L);
}


void CLAppDateTime::addDay(int nDay)
{
	mCurrentTime.tv_sec += (nDay * 86400L);
}

void CLAppDateTime::addHour(int nHour)
{
	mCurrentTime.tv_sec += (nHour * 3600L);
}

void CLAppDateTime::addMinute(int nMinute)
{
	mCurrentTime.tv_sec += (nMinute * 60L);
}

void CLAppDateTime::addSecond(int nSecond)
{
	mCurrentTime.tv_sec += nSecond;
}

void CLAppDateTime::addMilisecond(int nMilisecond)
{
	mCurrentTime.tv_usec += (nMilisecond * 1000L);

	if(mCurrentTime.tv_usec >= 0)
	{
		mCurrentTime.tv_sec += (mCurrentTime.tv_usec / 1000000L);
		mCurrentTime.tv_usec %= 1000000L;
	}
	else
	{
		mCurrentTime.tv_sec += (mCurrentTime.tv_usec / 1000000L + 1);
		mCurrentTime.tv_usec += 1000000L;
	}
}


void CLAppDateTime::addMicrosecond(int nMicrosecond)
{
	mCurrentTime.tv_usec += nMicrosecond;

	if(mCurrentTime.tv_usec >= 0)
	{
		mCurrentTime.tv_sec += (mCurrentTime.tv_usec / 1000000L);
		mCurrentTime.tv_usec %= 1000000L;
	}
	else
	{
		mCurrentTime.tv_sec += (mCurrentTime.tv_usec / 1000000L + 1);
		mCurrentTime.tv_usec += 1000000L;
	}
}


long CLAppDateTime::getTotalSeconds()
{
	return mCurrentTime.tv_sec;
}

long CLAppDateTime::getTotalMiliseconds()
{
	return (mCurrentTime.tv_usec / 1000);
}

long CLAppDateTime::getTotalMicroseconds()
{
	return mCurrentTime.tv_usec;
}

string CLAppDateTime::toTimeString(const char* szFormat)
{
	if(szFormat == NULL)
	{
		return timeString(getTime(), "yyyy-mm-dd HH:MM:SS");
	}

	return timeString(getTime(), szFormat);
}

string CLAppDateTime::timeString(time_t t, const char* szFormat)
{
	struct tm tmc;
	gmtime_r(&t, &tmc);

	string szTmp = "";
	string szDate = szFormat;

	szTmp = CLAppString::format("%04d", tmc.tm_year + 1900);

	for(size_t i=0; i<szDate.size() - 3; i++)
	{
		if(szDate.substr(i, 4) == "yyyy")
		{
			szDate.replace(i, 4, szTmp);
		}
	}

	szTmp = CLAppString::format("%02d", (tmc.tm_year + 1900) % 100);

	for(size_t i=0; i<szDate.size() - 1; i++)
	{
		if(szDate.substr(i, 2) == "yy")
		{
			szDate.replace(i, 2, szTmp);
		}
	}

	szTmp = CLAppString::format("%02d", tmc.tm_mon + 1);

	for(size_t i=0; i<szDate.size() - 1; i++)
	{
		if(szDate.substr(i, 2) == "mm")
		{
			szDate.replace(i, 2, szTmp);
		}
	}

	szTmp = CLAppString::format("%02d", tmc.tm_mday);

	for(size_t i=0; i<szDate.size() - 1; i++)
	{
		if(szDate.substr(i, 2) == "dd")
		{
			szDate.replace(i, 2, szTmp);
		}
	}

	szTmp = CLAppString::format("%02d", tmc.tm_hour);

	for(size_t i=0; i<szDate.size() - 1; i++)
	{
		if(szDate.substr(i, 2) == "HH")
		{
			szDate.replace(i, 2, szTmp);
		}
	}

	szTmp = CLAppString::format("%02d", tmc.tm_min);

	for(size_t i=0; i<szDate.size() - 1; i++)
	{
		if(szDate.substr(i, 2) == "MM")
		{
			szDate.replace(i, 2, szTmp);
		}
	}

	szTmp = CLAppString::format("%02d", tmc.tm_sec);

	for(size_t i=0; i<szDate.size() - 1; i++)
	{
		if(szDate.substr(i, 2) == "SS")
		{
			szDate.replace(i, 2, szTmp);
		}
	}

	return szDate;
}


CLAppDateTime& CLAppDateTime::operator=(const CLAppDateTime& t)
{
	CLAppDateTime* pt = (CLAppDateTime*)&t;
	pt->getTime(&mCurrentTime);
	return *this;
}


CLAppDateTime& CLAppDateTime::operator = (const struct timeval& t)
{
	//mCurrentTime = t;
	memcpy(&mCurrentTime, (struct timeval*)&t, sizeof(struct timeval));
	return *this;
}


CLAppDateTime& CLAppDateTime::operator = (const time_t t)
{
	mCurrentTime.tv_sec = t;
	mCurrentTime.tv_usec = 0;
	return *this;
}


CLAppDateTime& CLAppDateTime::operator = (const struct tm& t)
{
	struct tm* pt = (struct tm*)&t;
	mCurrentTime.tv_sec = mktime(pt);
	mCurrentTime.tv_usec = 0;
	return *this;
}



CLAppDateTime& CLAppDateTime::operator += (const CLAppTimeSpan& ts)
{
	CLAppTimeSpan* pts = (CLAppTimeSpan*)&ts;
	struct timeval tmp1, tmp2;

	if(mCurrentTime.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec * (-1L);
	}

	if(pts->getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds() * (-1L);
	}

	tmp1.tv_sec += tmp2.tv_sec;
	tmp1.tv_usec += tmp2.tv_usec;

	if(tmp1.tv_usec < 0)
	{
		tmp1.tv_sec--;
		tmp1.tv_usec += 1000000L;
	}
	
	tmp1.tv_sec += (tmp1.tv_usec / 1000000L);
	tmp1.tv_usec %= 1000000L;

	if(tmp1.tv_sec < 0)
	{
		tmp1.tv_sec = 0;
		tmp1.tv_usec = 0;
	}

	mCurrentTime = tmp1;
	return *this;
}


CLAppDateTime& CLAppDateTime::operator -= (const CLAppTimeSpan& ts)
{
	CLAppTimeSpan* pts = (CLAppTimeSpan*)&ts;
	struct timeval tmp1, tmp2;

	if(mCurrentTime.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec * (-1L);
	}

	if(pts->getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds() * (-1L);
	}

	tmp1.tv_sec -= tmp2.tv_sec;
	tmp1.tv_usec -= tmp2.tv_usec;

	if(tmp1.tv_usec < 0)
	{
		tmp1.tv_sec--;
		tmp1.tv_usec += 1000000L;
	}

	tmp1.tv_sec += (tmp1.tv_usec / 1000000L);
	tmp1.tv_usec %= 1000000L;

	if(tmp1.tv_sec < 0)
	{
		tmp1.tv_sec = 0;
		tmp1.tv_usec = 0;
	}

	mCurrentTime = tmp1;
	return *this;
}


CLAppTimeSpan CLAppDateTime::operator-( CLAppDateTime time ) const
{
	struct timeval tmp, tmp1, tmp2;
	long sign = 1;

	if(mCurrentTime.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec * (-1L);
	}

	if(time.getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = time.getTotalSeconds();
		tmp2.tv_usec = time.getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = time.getTotalSeconds();
		tmp2.tv_usec = time.getTotalMicroseconds() * (-1L);
	}

	if(tmp1.tv_sec > tmp2.tv_sec ||
		(tmp1.tv_sec == tmp2.tv_sec && tmp1.tv_usec > tmp2.tv_usec))
	{
		tmp.tv_sec = tmp1.tv_sec - tmp2.tv_sec;
		tmp.tv_usec = tmp1.tv_usec - tmp2.tv_usec;
	}
	else
	{
		tmp.tv_sec = tmp2.tv_sec - tmp1.tv_sec;
		tmp.tv_usec = tmp2.tv_usec - tmp1.tv_usec;
		sign = -1;
	}

	if(tmp.tv_usec < 0)
	{
		tmp.tv_sec--;
		tmp.tv_usec += 1000000L;
	}

	tmp.tv_sec += (tmp.tv_usec / 1000000L);
	tmp.tv_usec %= 1000000L;

	tmp.tv_sec *= sign;

	return CLAppTimeSpan(tmp);
}


CLAppDateTime CLAppDateTime::operator-( CLAppTimeSpan span ) const
{
	struct timeval tmp, tmp1, tmp2;
	long sign = 1;

	if(mCurrentTime.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec * (-1L);
	}

	if(span.getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds() * (-1L);
	}

	if(tmp1.tv_sec > tmp2.tv_sec ||
		(tmp1.tv_sec == tmp2.tv_sec && tmp1.tv_usec > tmp2.tv_usec))
	{
		tmp.tv_sec = tmp1.tv_sec - tmp2.tv_sec;
		tmp.tv_usec = tmp1.tv_usec - tmp2.tv_usec;
	}
	else
	{
		tmp.tv_sec = tmp2.tv_sec - tmp1.tv_sec;
		tmp.tv_usec = tmp2.tv_usec - tmp1.tv_usec;
		sign = -1;
	}

	if(tmp.tv_usec < 0)
	{
		tmp.tv_sec--;
		tmp.tv_usec += 1000000L;
	}

	tmp.tv_sec += (tmp.tv_usec / 1000000L);
	tmp.tv_usec %= 1000000L;

	tmp.tv_sec *= sign;

	return CLAppDateTime(tmp);
}


CLAppDateTime CLAppDateTime::operator+( CLAppTimeSpan span ) const
{
	struct timeval tmp1, tmp2;

	if(mCurrentTime.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentTime.tv_sec;
		tmp1.tv_usec = mCurrentTime.tv_usec * (-1L);
	}

	if(span.getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds() * (-1L);
	}

	tmp1.tv_sec += tmp2.tv_sec;
	tmp1.tv_usec += tmp2.tv_usec;

	if(tmp1.tv_usec < 0)
	{
		tmp1.tv_sec--;
		tmp1.tv_usec += 1000000L;
	}

	tmp1.tv_sec += (tmp1.tv_usec / 1000000L);
	tmp1.tv_usec %= 1000000L;

	if(tmp1.tv_sec < 0)
	{
		tmp1.tv_sec = 0;
		tmp1.tv_usec = 0;
	}

	return CLAppDateTime(tmp1);
}


bool CLAppDateTime::operator==(CLAppDateTime t) const
{
	if(mCurrentTime.tv_sec == t.getTotalSeconds() &&
		mCurrentTime.tv_usec == t.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppDateTime::operator!=(CLAppDateTime t) const
{
	if(mCurrentTime.tv_sec != t.getTotalSeconds() ||
		mCurrentTime.tv_usec != t.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppDateTime::operator<(CLAppDateTime t) const
{
	if(mCurrentTime.tv_sec < t.getTotalSeconds()) 
		return true;
	else if(mCurrentTime.tv_sec == t.getTotalSeconds() &&
		mCurrentTime.tv_usec < t.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppDateTime::operator>(CLAppDateTime t) const
{
	if(mCurrentTime.tv_sec > t.getTotalSeconds()) 
		return true;
	else if(mCurrentTime.tv_sec == t.getTotalSeconds() &&
		mCurrentTime.tv_usec > t.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppDateTime::operator<=(CLAppDateTime t) const
{
	if(mCurrentTime.tv_sec < t.getTotalSeconds()) 
		return true;
	else if(mCurrentTime.tv_sec == t.getTotalSeconds() &&
		mCurrentTime.tv_usec <= t.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppDateTime::operator>=(CLAppDateTime t) const
{
	if(mCurrentTime.tv_sec > t.getTotalSeconds()) 
		return true;
	else if(mCurrentTime.tv_sec == t.getTotalSeconds() &&
		mCurrentTime.tv_usec >= t.getTotalMicroseconds())
		return true;

	return false;
}


CLAppTimeSpan::CLAppTimeSpan()
{
	mCurrentSpan.tv_sec = 0;
	mCurrentSpan.tv_usec = 0;
}


CLAppTimeSpan::CLAppTimeSpan(const CLAppTimeSpan& ts)
{
	CLAppTimeSpan* pts = (CLAppTimeSpan*)&ts;
	pts->getTimeSpan(&mCurrentSpan);
}


CLAppTimeSpan::CLAppTimeSpan(const struct timeval ts)
{
	mCurrentSpan = ts;
}

CLAppTimeSpan::~CLAppTimeSpan()
{
}


long CLAppTimeSpan::getTotalYears()
{
	return (mCurrentSpan.tv_sec / 31536000L);
}


long CLAppTimeSpan::getTotalDays()
{
	return (mCurrentSpan.tv_sec / 86400L);
}


long CLAppTimeSpan::getTotalHours()
{
	return (mCurrentSpan.tv_sec / 3600L);
}


long CLAppTimeSpan::getTotalMinutes()
{
	return (mCurrentSpan.tv_sec / 60L);
}


long CLAppTimeSpan::getTotalSeconds()
{
	return mCurrentSpan.tv_sec;
}

long CLAppTimeSpan::getTotalMiliseconds()
{
	return (mCurrentSpan.tv_usec / 1000);
}

long CLAppTimeSpan::getTotalMicroseconds()
{
	return mCurrentSpan.tv_usec;
}

void CLAppTimeSpan::addYear(int nYear)
{
	mCurrentSpan.tv_sec += (nYear * 31536000L);
}

void CLAppTimeSpan::addMonth(int nMonth)
{
	mCurrentSpan.tv_sec += (nMonth * 2592000L);
}


void CLAppTimeSpan::addDay(int nDay)
{
	mCurrentSpan.tv_sec += (nDay * 86400L);
}


void CLAppTimeSpan::addHour(int nHour)
{
	mCurrentSpan.tv_sec += (nHour * 3600L);
}


void CLAppTimeSpan::addMinute(int nMinute)
{
	mCurrentSpan.tv_sec += (nMinute * 60L);
}


void CLAppTimeSpan::addSecond(int nSecond)
{
	mCurrentSpan.tv_sec += nSecond;
}


void CLAppTimeSpan::addMilisecond(int nMilisecond)
{
	mCurrentSpan.tv_usec += (nMilisecond * 1000L);

	if(mCurrentSpan.tv_usec >= 0)
	{
		mCurrentSpan.tv_sec += (mCurrentSpan.tv_usec / 1000000L);
		mCurrentSpan.tv_usec %= 1000000L;
	}
	else
	{
		mCurrentSpan.tv_sec += (mCurrentSpan.tv_usec / 1000000L + 1);
		mCurrentSpan.tv_usec += 1000000L;
	}
}


void CLAppTimeSpan::addMicrosecond(int nMicrosecond)
{
	mCurrentSpan.tv_usec += nMicrosecond;

	if(mCurrentSpan.tv_usec >= 0)
	{
		mCurrentSpan.tv_sec += (mCurrentSpan.tv_usec / 1000000L);
		mCurrentSpan.tv_usec %= 1000000L;
	}
	else
	{
		mCurrentSpan.tv_sec += (mCurrentSpan.tv_usec / 1000000L + 1);
		mCurrentSpan.tv_usec += 1000000L;
	}
}


string CLAppTimeSpan::toString()
{
	return CLAppString::format("%ld.%06ld", 
		mCurrentSpan.tv_sec, mCurrentSpan.tv_usec);
}


struct timeval* CLAppTimeSpan::getTimeSpan(struct timeval* tv)
{
	if(tv != NULL)
	{
		*tv = mCurrentSpan;
	}

	return tv;
}


CLAppTimeSpan& CLAppTimeSpan::operator=(const CLAppTimeSpan& ts)
{
	CLAppTimeSpan* pts = (CLAppTimeSpan*)&ts;
	pts->getTimeSpan(&mCurrentSpan);
	return *this;
}


CLAppTimeSpan& CLAppTimeSpan::operator+=(const CLAppTimeSpan& ts)
{
	CLAppTimeSpan* pts = (CLAppTimeSpan*)&ts;
	struct timeval tmp1, tmp2;

	if(mCurrentSpan.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec * (-1L);
	}

	if(pts->getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds() * (-1L);
	}

	tmp1.tv_sec += tmp2.tv_sec;
	tmp1.tv_usec += tmp2.tv_usec;

	if(tmp1.tv_usec < 0)
	{
		tmp1.tv_sec--;
		tmp1.tv_usec += 1000000L;
	}

	tmp1.tv_sec += (tmp1.tv_usec / 1000000L);
	tmp1.tv_usec %= 1000000L;

	mCurrentSpan = tmp1;
	return *this;
}


CLAppTimeSpan& CLAppTimeSpan::operator-=(const CLAppTimeSpan& ts)
{
	CLAppTimeSpan* pts = (CLAppTimeSpan*)&ts;
	struct timeval tmp, tmp1, tmp2;
	long sign = 1;

	if(mCurrentSpan.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec * (-1L);
	}

	if(pts->getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = pts->getTotalSeconds();
		tmp2.tv_usec = pts->getTotalMicroseconds() * (-1L);
	}

	if(tmp1.tv_sec > tmp2.tv_sec ||
		(tmp1.tv_sec == tmp2.tv_sec && tmp1.tv_usec > tmp2.tv_usec))
	{
		tmp.tv_sec = tmp1.tv_sec - tmp2.tv_sec;
		tmp.tv_usec = tmp1.tv_usec - tmp2.tv_usec;
	}
	else
	{
		tmp.tv_sec = tmp2.tv_sec - tmp1.tv_sec;
		tmp.tv_usec = tmp2.tv_usec - tmp1.tv_usec;
		sign = -1;
	}

	if(tmp.tv_usec < 0)
	{
		tmp.tv_sec--;
		tmp.tv_usec += 1000000L;
	}

	tmp.tv_sec += (tmp.tv_usec / 1000000L);
	tmp.tv_usec %= 1000000L;

	tmp.tv_sec *= sign;

	mCurrentSpan = tmp;
	return *this;
}


CLAppTimeSpan CLAppTimeSpan::operator+(CLAppTimeSpan span) const
{
	struct timeval tmp1, tmp2;

	if(mCurrentSpan.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec * (-1L);
	}

	if(span.getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds() * (-1L);
	}

	tmp1.tv_sec += tmp2.tv_sec;
	tmp1.tv_usec += tmp2.tv_usec;

	if(tmp1.tv_usec < 0)
	{
		tmp1.tv_sec--;
		tmp1.tv_usec += 1000000L;
	}

	tmp1.tv_sec += (tmp1.tv_usec / 1000000L);
	tmp1.tv_usec %= 1000000L;

	if(tmp1.tv_sec < 0)
	{
		tmp1.tv_sec = 0;
		tmp1.tv_usec = 0;
	}

	return CLAppTimeSpan(tmp1);
}


CLAppTimeSpan CLAppTimeSpan::operator-(CLAppTimeSpan span) const
{
	struct timeval tmp, tmp1, tmp2;
	long sign = 1;

	if(mCurrentSpan.tv_sec >= 0)
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec;
	}
	else
	{
		tmp1.tv_sec = mCurrentSpan.tv_sec;
		tmp1.tv_usec = mCurrentSpan.tv_usec * (-1L);
	}

	if(span.getTotalSeconds() >= 0)
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds();
	}
	else
	{
		tmp2.tv_sec = span.getTotalSeconds();
		tmp2.tv_usec = span.getTotalMicroseconds() * (-1L);
	}

	if(tmp1.tv_sec > tmp2.tv_sec ||
		(tmp1.tv_sec == tmp2.tv_sec && tmp1.tv_usec > tmp2.tv_usec))
	{
		tmp.tv_sec = tmp1.tv_sec - tmp2.tv_sec;
		tmp.tv_usec = tmp1.tv_usec - tmp2.tv_usec;
	}
	else
	{
		tmp.tv_sec = tmp2.tv_sec - tmp1.tv_sec;
		tmp.tv_usec = tmp2.tv_usec - tmp1.tv_usec;
		sign = -1;
	}

	if(tmp.tv_usec < 0)
	{
		tmp.tv_sec--;
		tmp.tv_usec += 1000000L;
	}

	tmp.tv_sec += (tmp.tv_usec / 1000000L);
	tmp.tv_usec %= 1000000L;

	tmp.tv_sec *= sign;

	return CLAppTimeSpan(tmp);
}


bool CLAppTimeSpan::operator==(CLAppTimeSpan span) const
{
	if(mCurrentSpan.tv_sec == span.getTotalSeconds() &&
		mCurrentSpan.tv_usec == span.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppTimeSpan::operator!=(CLAppTimeSpan span) const
{
	if(mCurrentSpan.tv_sec != span.getTotalSeconds() ||
		mCurrentSpan.tv_usec != span.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppTimeSpan::operator>(CLAppTimeSpan span) const
{
	if(mCurrentSpan.tv_sec > span.getTotalSeconds()) 
		return true;
	else if(mCurrentSpan.tv_sec == span.getTotalSeconds() &&
		mCurrentSpan.tv_usec > span.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppTimeSpan::operator>=(CLAppTimeSpan span) const
{
	if(mCurrentSpan.tv_sec > span.getTotalSeconds()) 
		return true;
	else if(mCurrentSpan.tv_sec == span.getTotalSeconds() &&
		mCurrentSpan.tv_usec >= span.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppTimeSpan::operator<(CLAppTimeSpan span) const
{
	if(mCurrentSpan.tv_sec < span.getTotalSeconds()) 
		return true;
	else if(mCurrentSpan.tv_sec == span.getTotalSeconds() &&
		mCurrentSpan.tv_usec < span.getTotalMicroseconds())
		return true;

	return false;
}


bool CLAppTimeSpan::operator<=(CLAppTimeSpan span) const
{
	if(mCurrentSpan.tv_sec < span.getTotalSeconds()) 
		return true;
	else if(mCurrentSpan.tv_sec == span.getTotalSeconds() &&
		mCurrentSpan.tv_usec <= span.getTotalMicroseconds())
		return true;

	return false;
}

int CLAppDateTime::getMonthDays(int year, int month) 
{
	int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if(year >= 1970 && year < 2038 && month >= 1 && month <= 12)
	{
		if(((year%4 == 0) && (year%100 != 0)) || (year%400 == 0)) monthDays[1] = 29;
		else monthDays[1] = 28;

		return monthDays[month-1];
	}

	return 31;
}


bool CLAppDateTime::getRtc( void )
{
	struct tm        tm_src; //  = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	struct timeval   tv = { 0, 0 };


	tm_src.tm_year = 2015;
	tm_src.tm_mon  = 7; 
	tm_src.tm_mday = 1;
	tm_src.tm_hour = 1;
	tm_src.tm_min  = 1;
	tm_src.tm_sec  = 0;

	tv.tv_sec = mktime( &tm_src );
	tv.tv_usec = 100;	// �����ð� : 100 usec

	settimeofday ( &tv, NULL ); 
	return true;
}


struct timeval* CLAppDateTime::setSystemTime(struct timeval *ptv)
{
	if(ptv != NULL)
	{
		struct tm		tm;

		settimeofday(ptv, NULL);
		gmtime_r(&ptv->tv_sec, &tm);
	}
	return ptv;
}


time_t CLAppDateTime::setSystemTime(time_t& t)
{
	struct timeval tv = { t, 0 };
	setSystemTime(&tv);

	return t;
}


struct tm* CLAppDateTime::setSystemTime(struct tm* ptm)
{
	if(ptm != NULL)
	{
		struct timeval tv = { 0, 0 };
		tv.tv_sec = mktime(ptm);
		setSystemTime(&tv);
	}
	return ptm;
}


CLAppDateTime CLAppDateTime::getSystemTime()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	
	CLAppDateTime dt(tv);
	return dt;
}

void CLAppDateTime::setSystemTime(const CLAppDateTime& time)
{
	CLAppDateTime* ptime = (CLAppDateTime*)&time;
	char szBufs[32] = {0, };
	int ret = 0;

	memset(szBufs, 0, 32);	
	sprintf(szBufs, "date %02d%02d%02d%02d%04d.%02d", 
			(ptime->getMonth() + 1), ptime->getDay(),
			ptime->getHour(), ptime->getMinute(),
			ptime->getYear(), ptime->getSecond());
	ret = system(szBufs);
	if(ret == 0) { }
}
