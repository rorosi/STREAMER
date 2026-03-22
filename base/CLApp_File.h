/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_File.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_FILE_H_
#define _CLAPP_FILE_H_

#include <string>
#include <vector>
#include <map>
#include "CLApp_Time.h"

class CLAppFile
{
public:
	static const int begin;
	static const int current;
	static const int end;

	static const int modeRead;
	static const int modeWrite;
	static const int modeCreate;
	static const int modeTruncate;
	static const int modeExclude;
	static const int modeAppend;
	static const int modeSync;
	static const int modeDirect;
	static const int modeRdWr;
	//static const int modeBinary;
	//static const int modeText;

public:
	CLAppFile();
	CLAppFile(const char* szFilename, 
		int nMode = modeRead | modeWrite | modeCreate, int nAccess = 0755);
	virtual ~CLAppFile();

	bool open(const char* szFilename, int nMode, int nAccess = 0755);
	void close();
	
	int ioctl(const int command, void *args);
	ssize_t read(char* pBuffer, size_t nMaxLength);
	ssize_t write(const char* pBuffer, size_t nLength);
	off_t seek(off_t nOffsset, int nWhence);	
	void fsync();
	void fdatasync();
	size_t length();
	off_t position();
	
	static size_t length(const char* szFilename);
	static bool exists(const char* szFilename);
	static bool remove(const char* szFilename);
	static int  fileNCopy( const char* szDst, const char* szSrc,
						   int nBlockSize );
	static int  fileCopy( const char* szDst, const char* szSrc );
	static bool rename(const char* szOld, const char* szNew);
	static bool copy(const char* szSrc, const char* szDst);
	
	bool isOpened();
	bool haveIOError();

	int getFileHandle();
	const char* getFilename();
	CLAppDateTime getLastAccessTime();
	CLAppDateTime getLastModificationTime();
	CLAppDateTime getLastChangeTime();	
	
	string trimString(string str);
	void clear();
	bool load(string filename);
	bool getValue(string key, char* data);
	bool getValue(string key, string& data);
	
private:
	int			mhFile;
	std::string	mszFilename;

protected:	
	map<string, string>	msz_table;
};


class CLAppDirectory
{
public:
	CLAppDirectory();
	CLAppDirectory(const char* szDirname);
	virtual ~CLAppDirectory();

	static bool create(const char* szDirname, int mode = 0755);
	static bool exists(const char* szDirname);
	static bool remove(const char* szDirname);
	static bool rename(const char* szOld, const char* szNew);
	static int getEntryCount(const char* szDirname);

//protected:
	static bool recursive_rm(const char* szDirname);

private:
	std::string mszDirname;
};

#endif // _CLAPP_FILE_H_
