/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_File.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "CLApp_String.h"
#include "CLApp_File.h"


const int CLAppFile::begin = SEEK_SET;
const int CLAppFile::current = SEEK_CUR;
const int CLAppFile::end = SEEK_END;

const int CLAppFile::modeRead = 4; //O_RDONLY;
const int CLAppFile::modeWrite = O_WRONLY;
const int CLAppFile::modeCreate = O_CREAT;
const int CLAppFile::modeTruncate = O_TRUNC;
const int CLAppFile::modeExclude = O_EXCL;
const int CLAppFile::modeAppend = O_APPEND;
const int CLAppFile::modeSync = O_SYNC;
const int CLAppFile::modeDirect = O_DIRECT;
const int CLAppFile::modeRdWr = O_RDWR;
//const int CLAppFile::modeBinary = O_BINARY;
//const int CLAppFile::modeText = O_TEXT;


CLAppFile::CLAppFile() : mhFile(-1), mszFilename("")
{
}


CLAppFile::CLAppFile(const char* szFilename, int nMode, int nAccess) : 
mhFile(-1), mszFilename(szFilename)
{
	open(szFilename, nMode, nAccess);
}

CLAppFile::~CLAppFile()
{
	close();
}

bool CLAppFile::open(const char* szFilename, int nMode, int nAccess)
{
	if(isOpened() == false)
	{
		if(szFilename != NULL)
		{
			if((nMode & modeRead) && (nMode & modeWrite))
			{
				nMode -= modeRead;
				nMode -= modeWrite;
				nMode += O_RDWR;
			}
			else if(nMode & modeRead)
			{
				nMode -= modeRead;
			}

			mszFilename = szFilename;
			mhFile = ::open(mszFilename.c_str(), nMode, nAccess);
			if ( mhFile >= 0 )
			{
				return true;
			}
			else
			{
    			printf("Error, open fail %s(FD%d) err=%s(%d) \n",
					  mszFilename.c_str(), mhFile, strerror( errno ), errno );
			}
		}
	}

	return false;
}

void CLAppFile::close()
{
	if(isOpened())
	{
		//::fsync(mhFile);
		::close(mhFile);
		mszFilename = "";
		mhFile = -1;
	}
}

int CLAppFile::ioctl(const int command, void *args)
{
	return ::ioctl(mhFile, command, args);
}

ssize_t CLAppFile::read(char* pBuffer, size_t nMaxLength)
{
	ssize_t	nSize = 0;

	nSize = ::read( mhFile, pBuffer, nMaxLength );
	if ( nSize != (ssize_t) nMaxLength )
	{
	}

	return nSize;
}

ssize_t CLAppFile::write(const char* pBuffer, size_t nLength)
{
	ssize_t	nSize = 0;

	nSize = ::write(mhFile, pBuffer, nLength);
	if ( nSize < 0 )
	{
		printf("Failed to write, size=%ld:%ld %s(FD%d) err=%s(%d) \n",
			  nSize, nLength,
			  mszFilename.c_str(), mhFile, strerror( errno ), errno );
	}
	else if ( nSize == 0 )
	{
		;
	}
	else if ( nSize != (ssize_t) nLength )
	{
		printf("Error, different to size, size=%ld:%ld %s(FD%d) \n",
			  nSize, nLength, mszFilename.c_str(), mhFile );
	}

	return nSize;
}


off_t CLAppFile::seek(off_t nOffsset, int nWhence)
{
	return ::lseek(mhFile, nOffsset, nWhence);
}

void CLAppFile::fsync()
{
	int iRet;

	if ( isOpened() )
	{
		iRet = ::fsync( mhFile );
		if ( iRet < 0 )
		{
			printf("Failed to fsync, %s(FD%d) err=%s(%d) \n",
				  mszFilename.c_str(), mhFile, strerror( errno ), errno );
		}
	}
}

void CLAppFile::fdatasync()
{
	int iRet;

	if ( isOpened() )
	{
		iRet = ::fdatasync( mhFile );
		//::posix_fadvise(mhFile, 0,0, POSIX_FADV_DONTNEED  );	
		if ( iRet < 0 )
		{
			printf("Failed to fdatasync, %s(FD%d) err=%s(%d) \n",
				  mszFilename.c_str(), mhFile, strerror( errno ), errno );
		}
	}
}

size_t CLAppFile::length()
{
	if(isOpened())
	{
		struct stat s;

		if(::fstat(mhFile, &s) == 0)
		{
			return s.st_size;
		}
	}

	return 0;
}


size_t CLAppFile::length(const char* szFilename)
{
	struct stat s;

	if(::stat(szFilename, &s) == 0)	
	{
		return s.st_size;
	}

	return 0;
}


off_t CLAppFile::position()
{
	return ::lseek(mhFile, 0, SEEK_CUR);
}


bool CLAppFile::exists(const char* szFilename)
{
	struct stat s;

	if(::stat(szFilename, &s) == 0)	
	{
		if(S_ISREG(s.st_mode))
			return true;
	}

	return false;
}


bool CLAppFile::remove( const char* szFilename )
{
	int		iRetryCnt = 1;
	bool	bRet = false;

	do
	{
		if ( ::remove( szFilename ) < 0 )
		{
			printf("Failed to remove, file %s err=%s(%d) retry:%d \n",
				  szFilename, strerror( errno ), errno, iRetryCnt );
			bRet = false;
			break;
		}

		/* check removed file still exist */
		if ( ::access( szFilename, F_OK ) == 0 )
		{
			printf("Failed to remove, file(%s) still exist retry:%d \n",
				  szFilename, iRetryCnt );
			bRet = false;
		}
		else
		{
			bRet = true;
			break;
		}

		iRetryCnt++;
	}while ( iRetryCnt <= 3 );

	return bRet;
}


bool CLAppFile::rename(const char* szOld, const char* szNew)
{
	return (::rename(szOld, szNew) == 0);
}


bool CLAppFile::copy(const char* szSrc, const char* szDst)
{ 
    int src_fd, dst_fd; 
    struct stat srcstat; 
    int n; 
    char buf[1024] = {0x00,}; 
	size_t ret = 0;

    if ((src_fd = ::open(szSrc, modeRead)) < 0 ) 
    { 
		printf("Failed to open src file %s \n", szSrc);
		return false; 
    } 
 
    ::fstat(src_fd, &srcstat); 
 
    if (::access(szDst, F_OK) == 0) { 
        ::remove(szDst);
    } 
 
    if ((dst_fd = ::open(szDst, modeWrite|modeCreate, srcstat.st_mode)) < 0 ) 
    { 
        printf("Failed to open dst file %s \n", szDst);
		::close(src_fd); 
        return false; 
    } 
	
    while((n = ::read(src_fd, buf, 1024)) > 0){ 
        ret = ::write((int)dst_fd, buf, n); 
		if(ret == 0) { }
    } 
 
    ret = ::fchown(dst_fd, srcstat.st_uid, srcstat.st_gid); 
    ret = ::close(src_fd); 
    ret = ::close(dst_fd); 
	if(ret == 0) { }

    return true; 
} 

int CLAppFile::fileNCopy( const char* szDst, const char* szSrc, int nBlockSize )
{
	CLAppFile		fSrc, fDst;

	char*		buf = NULL;
	int			nSize;
	int			nReadSize, nWriteSize;
	int			iOff;
	int			iRet;
				

	buf = (char*) alloca( (size_t) nBlockSize + 8 );
	if ( buf == NULL )
	{
		printf("Failed to alloca, size:%d \n", nBlockSize );
		iRet = -1;
		goto ERR_RET;
	}

	memset( buf, 0xFF, nBlockSize + 8 );

	nWriteSize = 0;
	nReadSize  = nBlockSize;

	/* open source file */
	fSrc.open( szSrc, CLAppFile::modeRead , 0755 );
	if ( !fSrc.isOpened() )
	{
		printf( "Failed to open to source, %s \n", szSrc );
		iRet = -2;
		goto ERR_RET;
	}


	/* open destination file */
	fDst.open( szDst, CLAppFile::modeWrite, 0755 );
	if ( !fDst.isOpened() )
	{
		printf("Failed to open to destinate, %s \n", szDst );
		iRet = -3;
		goto ERR_RET;
	}

	while ( 1 )
	{
		nSize = 0;
		iOff  = nBlockSize - nReadSize;

		nSize = fSrc.read( &buf[ iOff ], nReadSize );
		if ( nSize < 0 )
		{
			printf( "Failed to read, file:%s size:%d err=%s(%d) \n",
				  fSrc.getFilename(), nSize, strerror( errno ), errno );
			iRet = -4;
			goto ERR_RET;
		}
		else if ( nSize == 0 )
		{
			/* EOF */
			break;
		}
		else
		{
			nWriteSize += nSize;
			nReadSize  -= nSize;
		}

		if ( nWriteSize < nBlockSize )
			continue;

		iOff = 0;
		while ( 1 )
		{
			nSize = 0;
			iOff  = nBlockSize - nWriteSize;

			nSize = fDst.write( &buf[ iOff ], nWriteSize );
			if ( nSize <= 0 )
			{
				printf("Failed to write, file:%s size:%d err=%s(%d) \n",
					  fDst.getFilename(), nSize, strerror( errno ), errno );
				iRet = -5;
				goto ERR_RET;

			}

			nWriteSize -= nSize;

			if ( nWriteSize <= 0 )
			{
				memset( buf, 0xFF, nBlockSize + 8 );

				nReadSize  = nBlockSize;
				nWriteSize = 0;

				fDst.fsync();

				break;
			}
		}
	}

	iRet = 0;

ERR_RET :

	if ( fSrc.isOpened() )
		fSrc.close();

	if ( fDst.isOpened() )
		fDst.close();

	return iRet;

} /* End of fileNCopy() */
 

int CLAppFile::fileCopy( const char* szDst, const char* szSrc )
{
	char	cmd[ 128 + 8 ];
	int		iRet = 0;


	memset( cmd, 0x00, 128 + 8 );
	sprintf( cmd, "cp %s %s", szSrc, szDst );

	iRet = ::system( cmd );
	if ( iRet < 0 )
	{
		printf("Failed to execute command, cmd=%s err=%s(%d) \n",
			  cmd, strerror( errno ), errno );

		return -1;
	}

	return 0;

} /* End of fileNCopy() */


bool CLAppFile::isOpened()
{
	return (mhFile >= 0);
}


int CLAppFile::getFileHandle()
{
	return mhFile;
}


const char* CLAppFile::getFilename()
{
	return mszFilename.c_str();
}


CLAppDateTime CLAppFile::getLastAccessTime()
{
	if(isOpened())
	{
		struct stat s;

		if(::fstat(mhFile, &s) == 0)
		{
			CLAppDateTime dt;
			dt.setTime(s.st_atime);
			return dt;
		}
	}

	return 0;
}

CLAppDateTime CLAppFile::getLastModificationTime()
{
	if(isOpened())
	{
		struct stat s;

		if(::fstat(mhFile, &s) == 0)
		{
			CLAppDateTime dt;
			dt.setTime(s.st_atime);
			return dt;
		}
	}

	return 0;
}


CLAppDateTime CLAppFile::getLastChangeTime()
{
	if(isOpened())
	{
		struct stat s;

		if(::fstat(mhFile, &s) == 0)
		{
			CLAppDateTime dt;
			dt.setTime(s.st_atime);
			return dt;
		}
	}

	return 0;
}


string CLAppFile::trimString(string str)
{
	string tmp = str;

	for(int i= 0; i< (int)tmp.size(); i++)
	{
		if(	tmp[i] == ' ' || tmp[i] == '\r' ||
			tmp[i] == '\n' ||tmp[i] == '\t' ) {
			tmp.erase(i, 1);
			i--;
		} else {
			break;
		}
	}

	for(int i = (int)tmp.size() - 1; i >= 0; i--)
	{
		if(	tmp[i] == ' ' || tmp[i] == '\r' ||
				tmp[i] == '\n'|| tmp[i] == '\t') {
			tmp.erase(i, 1);
		} else {
			break;
		}
	}

	return tmp;
}

void CLAppFile::clear()
{
	msz_table.clear();
}

bool CLAppFile::load(string filename)
{
	FILE* fid = NULL;
	char buffer[4096];
	string cont = "";
	vector<string> lines;
	vector<string>::iterator mi;

	
	fid = fopen(filename.c_str(), "r");
	if ( fid == NULL ) {
		return false;
	}
           
	while(fgets(buffer, 2048, fid) != NULL) {
		lines.push_back(buffer);
	}	
	
	mi = lines.begin();

	for(;mi != lines.end(); advance(mi, 1))
	{
		string line = *mi;

		string::size_type x = line.find(';', 0);

		if(x != string::npos) {
			if(x == 0) {
				line = "";
			} else {
				line = line.substr(0, x);
			}
		}
		
		x = line.find('=', 0);

		if(x != string::npos)
		{
			string key = trimString(line.substr(0, x));
			string value = trimString(line.substr(x+1));
			value = CLAppString::replace(value, "[%C%R]", "\r");
			value = CLAppString::replace(value, "[%L%F]", "\n");

			if(msz_table.find(key) != msz_table.end())
			{
				msz_table.insert(pair<string, string>(key, value));
			}
			else
			{
				msz_table[key] = value;
			}
		}
	}

	fclose(fid);

	return true;
}

bool CLAppFile::getValue(string key, char* data)
{
	if(msz_table.find(key) != msz_table.end())
	{
		strcpy(data, msz_table[key].c_str());
		return true;
	}

	return false;
}

bool CLAppFile::getValue(string key, string& data)
{
	if(msz_table.find(key) != msz_table.end())
	{
		data = msz_table[key];
		return true;
	}

	return false;
}

CLAppDirectory::CLAppDirectory() : mszDirname("")
{

}


CLAppDirectory::CLAppDirectory(const char* szDirname) : 
mszDirname(szDirname)
{

}

CLAppDirectory::~CLAppDirectory()
{
}


bool CLAppDirectory::create(const char* szDirname, int nMode)
{
	return (mkdir(szDirname, nMode) == 0);
}


bool CLAppDirectory::exists(const char* szDirname)
{
	struct stat s;

	if(::stat(szDirname, &s) == 0)	
	{
		if(S_ISDIR(s.st_mode))
			return true;
	}

	return false;
}

bool CLAppDirectory::remove(const char* szDirname)
{
	return (::rmdir(szDirname) == 0);
}

bool CLAppDirectory::rename(const char* szOld, const char* szNew)
{
	return (::rename(szOld, szNew) == 0);
}

bool CLAppDirectory::recursive_rm(const char* szDirname)
{
	struct dirent **items = NULL;
	int nCount, ret = 0;
	char szOldDir[256];
	struct stat fstat;
	char *szRet;

	szRet = getcwd(szOldDir, 256);
	if(szRet == NULL) {}

	if (chdir(szDirname) < 0)
	{
		return false;
	}

	nCount = scandir(".", &items, NULL, alphasort);

	if(nCount >= 0 && items != NULL)
	{
		for(int i=0; i< nCount; i++)
		{
			if ( strcmp(items[i]->d_name, ".") != 0 && strcmp(items[i]->d_name, "..") != 0)
			{ 
				lstat(items[i]->d_name, &fstat);

				if ((fstat.st_mode & S_IFDIR) == S_IFDIR)
				{
					if(rmdir(items[i]->d_name) != 0) 
						recursive_rm(items[i]->d_name);
				}
				else
				{
					unlink(items[i]->d_name);
				}
			}

			free(items[i]);
		}

		free(items);
	}

	ret = chdir(szOldDir);
	if(ret == 0) {}

	return (rmdir(szDirname) == 0);
}

int CLAppDirectory::getEntryCount(const char* szDirname)
{
	int nFileCount = -1, ret = 0;
	char buf[256];
	char *szRet;

	szRet = getcwd(buf, 256);
	if(szRet == NULL) {}

	if (chdir(szDirname) == 0)
	{
		DIR *dirp = NULL;
		struct dirent *dp = NULL;
	
		if ((dirp = opendir(szDirname)) == NULL) {
			printf("couldn't open %s \n",szDirname);       
			return nFileCount;
		}

		do {
			if ((dp = readdir(dirp)) != NULL) {				
				if ( (!strcmp(dp->d_name, ".")) || (!strcmp(dp->d_name, "..")) )	{					
					continue;
				}

				int flength = strlen(dp->d_name);

				if(flength < 5){
					CLAppFile::remove(dp->d_name);
				}else{	
					nFileCount ++;					
				}	
			}
		} while (dp != NULL);

		if(dirp){
			(void) closedir(dirp);
		}
	}

	ret = chdir(buf);
	if(ret == 0){}

	return nFileCount;
}
