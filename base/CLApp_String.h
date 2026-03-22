/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_String.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_STRING_H_
#define _CLAPP_STRING_H_

#include <string>
#include <vector>

using namespace std;

typedef vector<string> strlist;

class CLAppString
{
public:
	CLAppString() {}
	~CLAppString() {}

	static strlist tokenize(const string& text, const string& delimiters);
	static string format(const char* szFmt, ...);
	static string trimLeft(const string& text, char ch );
	static string trimLeft(const string& text, const string& ch_set = " \t\r\n");
	static string trimRight(const string& text, char ch );
	static string trimRight(const string& text, const string& ch_set = " \t\r\n");
	static string trim(const string& text, char ch);
	static string trim(const string& text, const string& ch_set = " \t\r\n");
	static string toUpper(const string& text);
	static string toLower(const string& text);
	static int strcpy(char* dest, const char* src, unsigned int max = 0);
	static string replace (const string& source, const string target, const string replacement);
};

#endif	//_CLAPP_STRING_H_
