/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_String.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h> 

#include "CLApp_String.h"


strlist CLAppString::tokenize(const string& text, const string& delimiters)
{
	vector<string> tokens;
	string::size_type lastPos = text.find_first_not_of(delimiters, 0);
	string::size_type pos = text.find_first_of(delimiters, lastPos);

	while(string::npos != pos || string::npos != lastPos)
	{
		tokens.push_back(text.substr(lastPos, pos - lastPos));
		if(pos == text.size()-1)
			tokens.push_back("");
		lastPos = text.find_first_not_of(delimiters, pos);
		pos = text.find_first_of(delimiters, lastPos);
	}
	return tokens;
}

string CLAppString::format(const char* szFmt, ...)
{
	string str;
	va_list args;
	va_start (args, szFmt);

	int		res;
	long	size = 256;

	do  
	{
		str.resize(size);
		str.begin();
		res = vsnprintf ((char*)str.c_str(), size, szFmt, args);
		size += size;
	} while (res == -1 && size < 32767);

	if ( res > 0 )	str.resize(res);
	else			str.erase(str.begin(), str.end());

	va_end (args);

	return str;
}


string CLAppString::trimLeft(const string& text, char ch )  
{  
	string str = text;
	str.erase(0, str.find_first_not_of(ch));
	return str;
}

string CLAppString::trimLeft(const string& text, const string& ch_set)
{ 
	string str = text;
	str.erase(0, str.find_first_not_of(ch_set));
	return str;
}

string CLAppString::trimRight(const string& text, char ch )  
{ 
	string str = text;
	str.erase( str.find_last_not_of(ch) + 1);
	return str;
}

string CLAppString::trimRight(const string& text, const string& ch_set)  
{
	string str = text;
	str.erase( str.find_last_not_of(ch_set) + 1);
	return str;
}

string CLAppString::trim(const string& text, char ch)
{
	return trimLeft(trimRight(text, ch), ch);
}

string CLAppString::trim(const string& text, const string& ch_set)
{
	return trimLeft(trimRight(text, ch_set), ch_set);
}

int CLAppString::strcpy(char* dest, const char* src, unsigned int max)
{
	if(src == NULL || dest == NULL) return -1;

	int len = strlen(src);

	if(max > 0 && (int)max < len) len = max;

	if(strncpy(dest, src, len) != 0) return -1;

	*(dest + len) = '\0';

	return 0;
}

string CLAppString::toUpper(const string& text)  
{  
	string str = text;
	int length = str.length();

	for(int i=0; i<length; i++)
	{
		if(str[i] >= 0x61 && str[i] <= 0x7A)
		{
			str[i] -= 0x20;
		}
	}
	return str;
}

string CLAppString::toLower(const string& text)  
{  
	string str = text;
	int length = str.length();

	for(int i=0; i<length; i++)
	{
		if(str[i] >= 0x41 && str[i] <= 0x5A)
		{
			str[i] += 0x20;
		}
	}
	return str;
}

string CLAppString::replace (const string& source, const string target, const string replacement)
{
   string str = source;
   string::size_type pos = 0,  // where we are now
                found;   // where the found data is
   if (target.size () > 0)  // searching for nothing will cause a loop
   {
      while ((found = str.find (target, pos)) != string::npos)
      {
       str.replace (found, target.size (), replacement);
       pos = found + replacement.size ();
      }
   }
   return str;
}
