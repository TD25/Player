#pragma once

#include "File.h"
#include "wx/vector.h"
#include <assert.h>
#include "MyException.h"
#include "wx/thread.h"
#include "wx/event.h"
#include "PlayerFrame.h"


class Playlist
{
private:
	wxString mName;
	wxVector<File*> mFiles;
	File * Find(wxString path)
	{
		for (int i = 0; i < mFiles.size(); i++)
		{
			if (mFiles[i]->GetFullPath() == path)
				return mFiles[i];
		}
		return NULL;
	}
	File * Find(File * file)
	{
		for (int i = 0; i < mFiles.size(); i++)
		{
			if (mFiles[i] == file)
				return mFiles[i];
		}
		return NULL;
	}
public:
	Playlist(wxString name) : mName(name)
	{}
	void Add(File * file)
	{
		assert(file != NULL); 
		if (Find(file) != NULL)
			return;		//maybe throw exception?
		if (!file->Exists())
			throw MyException("File doesn't exist (Playlist::Add())",
					MyException::FATAL_ERROR);	//add error handling 
		//	somewhere if not fatal
		mFiles.push_back(file);
	}
	const wxVector<File*> * GetFiles() const 
	{
		return &mFiles;
	}
	const File* GetFile(int id)
	{
		assert(id < mFiles.size());
		return mFiles[id];
	}
	const File* GetFile(wxString name)
	{
		return Find(name);
	}
	wxString GetName()
	{
		return mName;
	}
};

class MediaLibrary
{
private:
	wxVector<wxString> mExtensions;
	wxString mName;
	wxVector<Playlist> mPlaylists;
	int FindPlaylist(wxString & name)
	{
		for (int i = 0; i < mPlaylists.size(); i++)
		{
			if (mPlaylists[i].GetName() == name)
				return i;
		}
		return -1;
	}
public:
	MediaLibrary(wxString name) : mName(name)
	{
		AddPlaylist("all");
	}
	MediaLibrary(wxString name, wxString extensions[], int n) : 
		mName(name)
	{
		mExtensions.assign(&extensions[0], &extensions[n-1]);
		AddPlaylist("all");
	}
	void AddExtension(wxString extension)
	{
		mExtensions.push_back(extension);
	}
	void AddExtensions(wxString extensions[], int n)
	{
		for (int i = 0; i < n; i++)
			AddExtension(extensions[i]);
	}
	void AddPlaylist(wxString name)
	{
		if (FindPlaylist(name) > 0)
			throw MyException(
					"There already is a playlist with this name", 
					MyException::NOT_FATAL);
		mPlaylists.push_back(name);
	}
	void AddFile(File * file)
	{
		mPlaylists[0].Add(file);
	}
	void AddFileToPlaylist(File * file, wxString playlistName)
	{
		mPlaylists[0].Add(file);
		int ind = FindPlaylist(playlistName);
		assert(ind > -1);
		mPlaylists[ind].Add(file);
	}
	const Playlist * GetPlaylist(wxString name)		
	{
		int ind = FindPlaylist(name);
		assert(ind > -1);
		return &mPlaylists[ind];
	}
	const Playlist * GetPlaylist(int ind)
	{
		return &mPlaylists[ind];
	}

	bool IsRightExtension(wxString ext)
	{
		for (int i = 0; i < mExtensions.size(); i++)
		{
			if (ext == mExtensions[i])
				return true;
		}
		return false;
	}

	wxString GetName()
	{
		return mName;
	}

};



// declare a new type of event, to be used by our SearcherThread class:
wxDECLARE_EVENT(EVT_SEARCHER_COMPLETE, wxThreadEvent);
wxDECLARE_EVENT(EVT_SEARCHER_UPDATE, ListUpdateEv);

class FileManager 
{
private:
	wxVector<File> mFiles;	//all files found
	wxVector<MediaLibrary> mLibs;
	int FindLib(const wxString & name);
	PlayerFrame * mHandlerFrame;

	class SearcherThread : public wxThread, public wxDirTraverser
	{
	private:
		FileManager * mFManager;
		//protects mFManager	
		wxCriticalSection mFManagerCS;
		wxDir mDir;
		PlayerFrame * mHandlerFrame;		
		virtual ExitCode Entry();
		int mSearchStage;
		wxString mFirstDir, mSecondDir;
	public:
		SearcherThread(FileManager * fMan, PlayerFrame *handler);
		~SearcherThread();
		virtual wxDirTraverseResult OnFile(const wxString& filename);
		virtual wxDirTraverseResult OnDir(const wxString& dirname);

	};
	SearcherThread * mpThread;
	wxCriticalSection mThreadCS;
	friend SearcherThread;
	//on these new found files events are sent
	wxString mCurrLib, mCurrPlaylist;
public:
	FileManager(PlayerFrame * frame = NULL) :
		mHandlerFrame(frame), mCurrLib(""), mCurrPlaylist("") {}
	void SetFrame(PlayerFrame * frame) {mHandlerFrame = frame;}
	void AddLib(MediaLibrary lib);
	void AddLib(const wxString & name);
	void AddLibs(MediaLibrary libs[], int n);
	//names represents playlist on which new files events are sent to frame
	//empty string means sent event on all playlists (not on all files though)
	void Search(wxString libName = "", wxString playlistName = "");
	void StopSearch();	//always call this on exiting the program
	void SetCurrPlaylist(wxString libName, wxString playlistName)
	{
		mCurrLib = libName;
		mCurrPlaylist = playlistName;
	}
	const Playlist * GetPlaylist(const wxString & libName, 
			const wxString & playlistName);
	//looks if extension of file is right
	//returns lib index
	int FromLib(const File & file);

};
