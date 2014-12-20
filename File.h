#pragma once
#include "wx/wx.h"
#include "wx/dir.h"
#include "wx/filename.h"

class File : public wxFileName
{
public:
	File(wxString path) : wxFileName(path) 
	{
	}
	File(wxFileName & filename) : wxFileName(filename)
	{
	}
	File() : wxFileName() 
	{}
	bool operator==(File & file2)
	{
		return (GetFullPath() == file2.GetFullPath());
	}
	bool operator!=(File & file2)
	{
		return (GetFullPath() != file2.GetFullPath());
	}
};

