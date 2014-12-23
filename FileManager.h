#pragma once

#include "wx/vector.h"
#include <assert.h>
#include "Common.h"
#include "wx/thread.h"
#include "wx/event.h"
#include "PlayerFrame.h"
#include "wx/filename.h"
#include "wx/dir.h"


class Playlist
{
private:
	wxString mName;
	wxVector<wxFileName*> mFiles;
	wxFileName * Find(wxString path) const
	{
		for (int i = 0; i < mFiles.size(); i++)
		{
			if (mFiles[i]->GetFullPath() == path)
				return mFiles[i];
		}
		return NULL;
	}
	wxFileName * Find(wxFileName * file) const 
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
	void Add(wxFileName * file)
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
	const wxVector<wxFileName*> * GetFiles() const 
	{
		return &mFiles;
	}
	const wxFileName* GetFile(const int & id) const 
	{
		assert(id < mFiles.size());
		return mFiles[id];
	}
	const wxFileName* GetFile(wxString name) const
	{
		return Find(name);
	}
	wxString GetName() const 
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
	int FindPlaylist(wxString & name) const
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
	void AddFile(wxFileName * file)
	{
		mPlaylists[0].Add(file);
	}
	void AddFileToPlaylist(wxFileName * file, wxString playlistName)
	{
		mPlaylists[0].Add(file);
		int ind = FindPlaylist(playlistName);
		assert(ind > -1);
		mPlaylists[ind].Add(file);
	}
	const Playlist * GetPlaylist(wxString name)	const
	{
		int ind = FindPlaylist(name);
		assert(ind > -1);
		return &mPlaylists[ind];
	}
	const Playlist * GetPlaylist(int ind) const
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

	wxString GetName() const
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
	wxVector<wxFileName*> mFiles;	//all files found
	wxVector<MediaLibrary> mLibs;
	int FindLib(const wxString & name) const;
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
		bool mStopped;
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
			const wxString & playlistName) const;
	//looks if extension of file is right
	//returns lib index
	int FromLib(const wxFileName & file);
	const wxFileName * GetFile(const wxString & libName, 
			const wxString & plName, const int & id) const;
	~FileManager();
};
