#pragma once
#include "wx/filename.h"
#include "Common.h"
#include "wx/vector.h"
#include "wx/string.h"
#include <fileref.h>

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
	virtual ~File() = 0;
};

template <typename T>	//T - file type
class FileT : public File
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
	FileT() : File(), mColContents(mColCount)
	{
	}
	FileT(const wxString & filename) : File(filename), 
		mColContents(mColCount)
	{
	}
	FileT(const wxFileName & wxfilename) : File(wxfilename),
		mColContents(mColCount)	{}
	virtual ~FileT()
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
			throw MyException("FileT::GetCol(): too big argument passed",
					MyException::FATAL_ERROR);
		return mColumns[n];
	}
	virtual int GetColSize(const int & n) const 
	{
		if (n >= mColCount)
			throw MyException("FileT::GetCol(): too big argument passed",
					MyException::FATAL_ERROR);
		return mColSizes[n];
	}

};

class MusicFile : public FileT<MusicFile>
{
private:
	TagLib::FileRef mTagFile;
protected:
	virtual void CollectInfo();
public:
	MusicFile() : FileT() {}
	MusicFile(const wxString & filename) : FileT(filename),
		mTagFile(filename.c_str())
	{
		CollectInfo();
	}
	MusicFile(const wxFileName & wxfilename) : FileT(wxfilename),
		mTagFile(wxfilename.GetFullPath())
	{
		CollectInfo();
	}
	virtual ~MusicFile() {}
};

