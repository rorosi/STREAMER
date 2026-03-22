/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Time.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_TIME_H_
#define _CLAPP_TIME_H_

#include <string>

using namespace std;

class CLAppTimeSpan;

class CLAppDateTime 
{
public:
	CLAppDateTime();
	CLAppDateTime(const CLAppDateTime& t);
	CLAppDateTime(const struct timeval& t);
	CLAppDateTime(const long tv_sec, const long tv_usec);
	CLAppDateTime(const time_t t);
	CLAppDateTime(const struct tm& t);
	CLAppDateTime(const int nYear, const int nMonth, const int nDay);
	CLAppDateTime(const int nYear, const int nMonth, const int nDay, 
		const int nHour, const int nMinute, const int nSecond);
	CLAppDateTime(const int nYear, const int nMonth, const int nDay,
		const int nHour, const int nMinute, const int nSecond, 
		const int nMilisecond);
	CLAppDateTime(const int nYear, const int nMonth, const int nDay,
		const int nHour, const int nMinute, const int nSecond, 
		const int nMilisecond, const int nMicrosecond);
	~CLAppDateTime();
	
	void setUserTime(const int nYear, const int nMonth, const int nDay,
						 const int nHour, const int nMinute, const int nSecond, 
						 const int nMilisecond, const int nMicrosecond);
	void setTime(const int nYear, const int nMonth, const int nDay,
		const int nHour, const int nMinute, const int nSecond, 
		const int nMilisecond, const int nMicrosecond);
	void setTime(struct timeval* pt);
	void setTime(struct tm* pt);
	void setTime(time_t t);
	struct timeval *getTime(struct timeval* pt);
	struct tm *getTime(struct tm* pt);
	time_t getTime();

	CLAppDateTime getFirstTimeOfMonth();
	CLAppDateTime getLastTimeOfMonth();
	CLAppDateTime getFirstTimeOfYear();
	CLAppDateTime getLastTimeOfYear();
	CLAppDateTime getFirstTimeOfDay();
	CLAppDateTime getLastTimeOfDay();

	int getYear();
	int getMonth();
	int getDay();
	int getHour();
	int getMinute();
	int getSecond();
	int getDayOfWeek();
	int getMilisecond();
	int getMicrosecond();

	long getTotalSeconds();
	long getTotalMiliseconds();
	long getTotalMicroseconds();

	void addYear(int nYear);
	void addMonth(int nMonth);
	void addDay(int nDay);
	void addHour(int nHour);
	void addMinute(int nMinute);
	void addSecond(int nSecond);
	void addMilisecond(int nMilisecond);
	void addMicrosecond(int nMicrosecond);

	static CLAppDateTime getSystemTime();
	static void setSystemTime(const CLAppDateTime& time);
	static bool getRtc();
	static struct timeval* setSystemTime(struct timeval *ptv);
	static time_t setSystemTime(time_t& t);
	static struct tm* setSystemTime(struct tm* ptm);

	string toTimeString(const char* szFormat = NULL);

	CLAppDateTime& operator = (const CLAppDateTime& t);
	CLAppDateTime& operator = (const struct timeval& t);
	CLAppDateTime& operator = (const time_t t);
	CLAppDateTime& operator = (const struct tm& t);
	CLAppDateTime& operator += (const CLAppTimeSpan& ts);
	CLAppDateTime& operator -= (const CLAppTimeSpan& ts);

	CLAppTimeSpan operator-( CLAppDateTime time ) const;
	CLAppDateTime operator-( CLAppTimeSpan span ) const;
	CLAppDateTime operator+( CLAppTimeSpan span ) const;

	bool operator==(CLAppDateTime time) const;
	bool operator!=(CLAppDateTime time) const;
	bool operator<(CLAppDateTime time) const;
	bool operator>(CLAppDateTime time) const;
	bool operator<=(CLAppDateTime time) const;
	bool operator>=(CLAppDateTime time) const;

	static string timeString(time_t t, const char* szFormat);
	
	int getMonthDays(int year, int month); 

private:
	struct timeval		mCurrentTime;
};


class CLAppTimeSpan
{
public:
	CLAppTimeSpan();
	CLAppTimeSpan(const CLAppTimeSpan& ts);
	CLAppTimeSpan(const struct timeval ts);
	~CLAppTimeSpan();

	long getTotalYears();
	long getTotalDays();
	long getTotalHours();
	long getTotalMinutes();
	long getTotalSeconds();
	long getTotalMiliseconds();
	long getTotalMicroseconds();

	void addYear(int nYear);
	void addMonth(int nMonth);
	void addDay(int nDay);
	void addHour(int nHour);
	void addMinute(int nMinute);
	void addSecond(int nSecond);
	void addMilisecond(int nMilisecond);
	void addMicrosecond(int nMicrosecond);

	struct timeval* getTimeSpan(struct timeval* tv);

	string toString();

	CLAppTimeSpan& operator=(const CLAppTimeSpan& ts);
	CLAppTimeSpan& operator+=(const CLAppTimeSpan& ts);
	CLAppTimeSpan& operator-=(const CLAppTimeSpan& ts);

	CLAppTimeSpan operator+(CLAppTimeSpan span) const;
	CLAppTimeSpan operator-(CLAppTimeSpan span) const;

	bool operator==(CLAppTimeSpan span) const;
	bool operator!=(CLAppTimeSpan span) const;
	bool operator>(CLAppTimeSpan span) const;
	bool operator>=(CLAppTimeSpan span) const;
	bool operator<(CLAppTimeSpan span) const;
	bool operator<=(CLAppTimeSpan span) const;

private:
	struct timeval mCurrentSpan;
};

#endif // _CLAPP_TIME_H_
