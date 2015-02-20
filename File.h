#pragma once
#include "wx/filename.h"
#include "Common.h"
#include "wx/vector.h"
#include "wx/string.h"
#include "wx/thread.h"
#ifdef MEDIAINFO_DLL
	#include <MediaInfoDLL/MediaInfoDLL.h>
	#define MediaInfoNamespace MediaInfoDLL;
#else //MEDIAINFO_DLL
	#include <MediaInfo/MediaInfo.h>
	#define MediaInfoNamespace MediaInfoLib;
#endif	//MEDIAINFO_DLL
using namespace MediaInfoNamespace;

class File : public wxFileName
{
protected:
	virtual void CollectInfo(); //this should get all info about
		//this file and fill mColContents
	mutable wxCriticalSection mCS; //protects file data
	struct FileInfo
	{
		//info for list
		const wxString * mColumns;
		const int mColCount;
		const int * mColSizes;
		const wxString mType;
		FileInfo(const wxString & type, const wxString * columns, const int colCount,
			const int * colSizes = nullptr) :
			mType(type), mColumns(columns), mColCount(colCount), mColSizes(colSizes) {}
	};
	static FileInfo mInfo; //derived classes should declare their own 
	wxVector<wxString> mColContents;
	virtual FileInfo & GetFileInfo() const = 0 //and implement this function
	{
		return mInfo;
	}
public:
	File() : wxFileName() {}
	File(const wxString & filename) : wxFileName(filename) 
	{
		mColContents.resize(mInfo.mColCount);
		CollectInfo();
	}
	File(const wxFileName & wxfilename) : wxFileName(wxfilename) 
	{
		mColContents.resize(mInfo.mColCount);
		CollectInfo();
	}
	int GetColCount() const
	{
		return GetFileInfo().mColCount;
	}
	virtual wxString GetColContent(const int & colNo) const
	{
		if (colNo >= mColContents.size())
			throw MyException("Too big colNo in File::GetColContent()",
					MyException::FATAL_ERROR);
		wxCriticalSectionLocker locker(mCS);
		return mColContents[colNo];
	}
	virtual wxVector<wxString> GetColNames() const
	{
		FileInfo & info = GetFileInfo();
		wxVector<wxString> v(info.mColumns, info.mColumns + info.mColCount);
		return v;
	}
	virtual wxString GetCol(const int & n) const
	{
		FileInfo & info = GetFileInfo();
		if (n >= info.mColCount)
			throw MyException("MediaFileT::GetCol(): too big argument passed",
					MyException::FATAL_ERROR);
		return info.mColumns[n];
	}
	virtual int GetColSize(const int & n) const
	{
		FileInfo & i = GetFileInfo();
		if (n >= i.mColCount)
			throw MyException("MediaFileT::GetCol(): too big argument passed",
					MyException::FATAL_ERROR);
		return i.mColSizes[n];
	}
	virtual wxString GetType() const
	{
		return GetFileInfo().mType;
	}
	virtual ~File() = 0;
};

class MediaFile : public File
{
protected:
	static MediaInfo mMInfoHandle;
	mutable wxCriticalSection mMInfoCS;
	static FileInfo mMedInfo;
	virtual FileInfo & GetFileInfo() const = 0
	{
		return mMedInfo;
	}
	virtual void CollectInfo() = 0;
	void StoreTitleAndTime(); //mMInfoHandle has to have opened file
public:
	MediaFile();
	MediaFile(const wxString & filename);
	MediaFile(const wxFileName & wxfilename);
	virtual wxString GetLengthStr() const
	{
		FileInfo & i = GetFileInfo();
		wxCriticalSectionLocker locker(mCS);
		if (mColContents[1].size() <= 0)
			return wxString("0.0");
		return mColContents[1];	
	}
	wxFileOffset GetLength()const; //in miliseconds
	virtual wxString GetTitle() const;
	virtual wxString GetArtist() const = 0;
};

class MusicFile : public MediaFile
{
protected:
	virtual void CollectInfo();
	static FileInfo mMusicInfo;
	virtual FileInfo & GetFileInfo() const
	{
		return mMusicInfo;
	}
public:
	MusicFile() : MediaFile() {}
	MusicFile(const wxString & filename) : MediaFile(filename)
	{
		mColContents.resize(GetColCount());
		CollectInfo();
	}
	MusicFile(const wxFileName & wxfilename) : MediaFile(wxfilename)
	{
		mColContents.resize(GetColCount());
		CollectInfo();
	}
	wxString GetArtist() const
	{
		FileInfo & i = GetFileInfo();
		wxCriticalSectionLocker locker(mCS);
		return mColContents[2];
	}
	virtual ~MusicFile() {}
};

class VideoFile : public MediaFile
{
protected:
	virtual void CollectInfo();
	static FileInfo mVidInfo;
	virtual FileInfo & GetFileInfo() const
	{
		return mVidInfo;
	}

public:
	VideoFile() {}
	VideoFile(const wxString & filename) : MediaFile(filename) 
	{
		mColContents.resize(GetColCount());
		CollectInfo();
	}
	VideoFile(const wxFileName & wxfilename) : MediaFile(wxfilename) 
	{
		mColContents.resize(GetColCount());
		CollectInfo();
	}
	wxString GetArtist() const
	{
		return wxString("");
	}
};
