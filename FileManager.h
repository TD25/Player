#pragma once

#include "wx/vector.h"
#include <assert.h>
#include "Common.h"
#include "wx/thread.h"
#include "wx/event.h"
#include "PlayerFrame.h"
#include "wx/filename.h"
#include "wx/dir.h"
#include "File.h"
#include "wx/msgqueue.h"

class Playlist
{
private:
	wxString mName;
	mutable wxCriticalSection mFilesCS;
	wxVector<File*> mFiles;
	File * Find(wxString path) const
	{
		for (int i = 0; i < mFiles.size(); i++)
		{
			if (mFiles[i]->GetFullPath() == path)
				return mFiles[i];
		}
		return NULL;
	}
	File * Find(File * file) const 
	{
		wxCriticalSectionLocker l(mFilesCS);
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
	Playlist(const Playlist & p) : mName(p.mName), mFiles(p.mFiles)
	{}
	void Add(File * file)
	{
		assert(file != NULL); 
		wxCriticalSectionLocker l(mFilesCS);
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
		wxCriticalSectionLocker l(mFilesCS);
		return &mFiles;
	}
	const File* GetFile(const int & id) const 
	{
		wxCriticalSectionLocker l(mFilesCS);
		assert(id < mFiles.size());
		return mFiles[id];
	}
	const File* GetFile(wxString name) const
	{
		//critical section locked in Find()
		return Find(name);
	}
	wxString GetName() const 
	{
		return mName;
	}
	wxVector<long> FindFiles(const wxString & mask) const;
};

class MediaLibrary
{
private:
	wxVector<wxString> mExtensions;
	wxString mName;
	mutable wxCriticalSection mPlaylistsCS;
	wxVector<Playlist> mPlaylists;
	int FindPlaylist(wxString & name) const
	{
		wxCriticalSectionLocker l(mPlaylistsCS);
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
		mExtensions.assign(&extensions[0], &extensions[n]);
		AddPlaylist("all");
	}
	MediaLibrary(const MediaLibrary& m) : mExtensions(m.mExtensions), mName(m.mName),
		mPlaylists(m.mPlaylists) {}
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
		wxCriticalSectionLocker l(mPlaylistsCS);
		mPlaylists.push_back(name);
	}
	void AddFile(File * file)
	{
		wxCriticalSectionLocker l(mPlaylistsCS);
		mPlaylists[0].Add(file);
	}
	void AddFileToPlaylist(File * file, wxString playlistName)
	{
		wxCriticalSectionLocker l(mPlaylistsCS);
		mPlaylists[0].Add(file);
		int ind = FindPlaylist(playlistName);
		assert(ind > -1);
		mPlaylists[ind].Add(file);
	}
	const Playlist * GetPlaylist(wxString name)	const
	{
		//critical section inside
		int ind = FindPlaylist(name);
		assert(ind > -1);
		return &mPlaylists[ind];
	}
	const Playlist * GetPlaylist(int ind) const
	{
		wxCriticalSectionLocker l(mPlaylistsCS);
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

class FileManager 
{
private:
	mutable wxCriticalSection mFilesCS;
	mutable wxCriticalSection mLibsCS;
	wxVector<File*> mFiles;	//all files found
	wxVector<MediaLibrary> mLibs;
	int FindLib(const wxString & name) const;
	PlayerFrame * mHandlerFrame;

	class SearcherThread;
	enum State //for communicating of threads
	{
		WORKING,
		DONE,
		TERMINATE
	};
	struct FileEvent
	{
		wxString mType;
		wxString mPath;
		int mInd; //index in file manager
		FileEvent(wxString type, wxString path, int ind) : mType(type), mPath(path),
			mInd(ind) {}
		FileEvent() : mInd(-1) {}
		FileEvent(const FileEvent & ev) : mType(ev.mType), mPath(ev.mPath), 
			mInd(ev.mInd) {}
	};
	class AnalyserThread : public wxThread
	{
	private:
		wxMessageQueue<FileEvent> * mQueue; //holds paths waiting for analysing
		FileManager * mFManager;
		PlayerFrame * mHandlerFrame;
		wxCriticalSection * mStateCS;
		State * mState;
		virtual ExitCode Entry();
		void Signal(State st)
		{
			wxCriticalSectionLocker enter(*mStateCS);
			*mState = st;
		}
	public:
		AnalyserThread(wxMessageQueue<FileEvent> * msgQueue,
			FileManager * fMan, State * state, wxCriticalSection* stateCS, PlayerFrame * frame);
		AnalyserThread(const AnalyserThread & a) = delete;
		~AnalyserThread() {}
	};

	class SearcherThread : public wxThread, public wxDirTraverser
	{
	private:
		FileManager * mFManager;
		wxDir mDir;
		PlayerFrame * mHandlerFrame;		
		virtual ExitCode Entry();
		bool mStopped;
		wxMessageQueue<FileEvent> mQueue;
		SearcherThread * mSearcherThread;
		AnalyserThread mThread;
		wxCriticalSection mThreadCS;
		wxCriticalSection mStateCS;
		State mState;
		void SignalAndWait();
		void StartAnalyserThread();
		void StopAnalyserThread();
	public:
		SearcherThread(FileManager * fMan, PlayerFrame *handler);
		SearcherThread(const SearcherThread & t) = delete;
		~SearcherThread();
		virtual wxDirTraverseResult OnFile(const wxString& filename);
		virtual wxDirTraverseResult OnDir(const wxString& dirname);
	};
	SearcherThread * mpThread;
	wxCriticalSection mThreadCS;
	friend SearcherThread;
	//on these new found files events are sent
	wxString mCurrLib, mCurrPlaylist;
protected:
	void AddFile(File * file)
	{
		assert(file != nullptr);
		wxCriticalSectionLocker l(mFilesCS);
		mFiles.push_back(file);
	}
public:
	FileManager(PlayerFrame * frame = NULL) :
		mHandlerFrame(frame), mCurrLib(""), mCurrPlaylist(""), 
		mpThread(nullptr) {}
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
	int FromLib(const wxString & file);
	const File * GetFile(const wxString & libName, 
			const wxString & plName, const int & id) const;
	//searches for files in playlist and returns indexes
	wxVector<long> FindFilesInPlaylist(const wxString & libName,
			const wxString & plName, const wxString & mask) const;
	bool IsSearching()
	{
		if (mpThread == nullptr)
			return false;
		return mpThread->IsAlive();
	}
	int GetLibCount() const
	{
		return mLibs.size();
	}
	bool IsFound(const wxFileName & filename) const;
	~FileManager();
};

wxString GetExtension(const wxString & str);