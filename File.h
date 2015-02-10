#pragma once
#include "wx/filename.h"
#include "Common.h"
#include "wx/vector.h"
#include "wx/string.h"
#ifdef MEDIAINFO_DLL
	#include <MediaInfoDLL/MediaInfoDLL.h>
	#define MediaInfoNamespace MediaInfoDLL;
#else //MEDIAINFO_DLL
	#include <MediaInfo/MediaInfo.h>
	#define MediaInfoNamespace MediaInfoLib;
#endif	//MEDIAINFO_DLL
using namespace MediaInfoNamespace;

//Curiously recurring template pattern:

class File : public wxFileName
{
protected:
	virtual void CollectInfo() = 0; //this should get all info about
		//this file and fill mColContents
public:
	File() : wxFileName() {}
	File(const wxString & filename) : wxFileName(filename) {}
	File(const wxFileName & wxfilename) : wxFileName(wxfilename) {}
	virtual int GetColCount() const = 0;
	virtual wxString GetColContent(const int & colNo) const = 0;
	virtual wxVector<wxString> GetColNames() const = 0;
	virtual wxString GetCol(const int & n) const = 0;
	virtual int GetColSize(const int & n) const = 0;
	virtual wxString GetType() const = 0;
	virtual ~File() = 0;
};

class MediaFile : public File
{
protected:
	static MediaInfo mMInfoHandle;
public:
	MediaFile() {}
	MediaFile(const wxString & filename) : File(filename) {}
	MediaFile(const wxFileName & wxfilename) : File(wxfilename) {}
	virtual wxString GetLengthStr() const = 0; //length may be stored in
										//column contents
	virtual wxFileOffset GetLength() const; //in miliseconds
};

template <typename T>	//T - file type
class MediaFileT : public MediaFile
{
protected:
	//info for list
	static const wxString * mColumns;
	static const int mColCount;
	static const int * mColSizes;
	wxVector<wxString> mColContents;
	virtual void CollectInfo()
	{
		mColContents[0] = GetName();
	}
public:
	MediaFileT() : MediaFile(), mColContents(mColCount)
	{
	}
	MediaFileT(const wxString & filename) : MediaFile(filename), 
		mColContents(mColCount)
	{
	}
	MediaFileT(const wxFileName & wxfilename) : MediaFile(wxfilename),
		mColContents(mColCount)	{}
	virtual ~MediaFileT()
	{
	}
	virtual int GetColCount() const { return mColCount; }
	virtual wxString GetColContent(const int & colNo) const 
	{
		if (colNo >= mColCount)
			throw MyException("Too big colNo in File::GetColContent()",
					MyException::FATAL_ERROR);
		return mColContents[colNo];
	}
	virtual wxVector<wxString> GetColNames() const
	{
		wxVector<wxString> cols(mColumns, mColumns+mColCount);
		return cols;
	}
	virtual wxString GetCol(const int & n) const 
	{
		if (n >= mColCount)
			throw MyException("MediaFileT::GetCol(): too big argument passed",
					MyException::FATAL_ERROR);
		return mColumns[n];
	}
	virtual int GetColSize(const int & n) const 
	{
		if (n >= mColCount)
			throw MyException("MediaFileT::GetCol(): too big argument passed",
					MyException::FATAL_ERROR);
		return mColSizes[n];
	}

	virtual wxString GetTitle() const //if you don't want 
									//track number in the title
	{
		return mColContents[0];
	}
};

class MusicFile : public MediaFileT<MusicFile>
{
protected:
	virtual void CollectInfo();
public:
	MusicFile() : MediaFileT() {}
	MusicFile(const wxString & filename) : MediaFileT(filename)
	{
		CollectInfo();
	}
	MusicFile(const wxFileName & wxfilename) : MediaFileT(wxfilename)
	{
		CollectInfo();
	}
	virtual wxString GetLengthStr() const
	{
		if (mColContents[1].size() <= 0)
			return wxString("0.0");
		return mColContents[1];	
	}
	wxString GetArtist() const
	{
		return mColContents[2];
	}
	virtual wxString GetType() const
	{
		return "Music";
	}
	virtual ~MusicFile() {}
};

class VideoFile : public MediaFileT<VideoFile>
{
protected:
	virtual void CollectInfo();
public:
	VideoFile() {}
	VideoFile(const wxString & filename) : MediaFileT(filename) 
	{
		CollectInfo();
	}
	VideoFile(const wxFileName & wxfilename) : MediaFileT(wxfilename) 
	{
		CollectInfo();
	}
	virtual wxString GetLengthStr() const
	{
		if (mColContents[1].size() <= 0)
			return wxString("0.0");
		return mColContents[1];	
	}
	virtual wxString GetType() const
	{
		return "Video";
	}
};
