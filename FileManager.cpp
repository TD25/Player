#include "FileManager.h"
#include "wx/event.h"
#include "PlayerFrame.h"
#include "wx/log.h"
#ifdef __WINDOWS__
	#include <windows.h>
	#include <string>
#endif

void FileManager::AddLib(MediaLibrary lib)
{
	if (FindLib(lib.GetName()) != -1)
		throw MyException("There already exists library with this name",
				MyException::NOT_FATAL);
	wxCriticalSectionLocker l(mLibsCS);
	mLibs.push_back(lib);
}

void FileManager::AddLib(const wxString & name)
{
	if (FindLib(name) != -1)
		throw MyException("There already exist library with this name",
				MyException::NOT_FATAL);
	wxCriticalSectionLocker l(mLibsCS);
	mLibs.push_back(name);
}

void FileManager::AddLibs(MediaLibrary libs[], int n)
{
	for (int i = 0; i < n; i++)
	{
		AddLib(libs[i]);
	}
}

int FileManager::FindLib(const wxString & name) const
{
	wxCriticalSectionLocker l(mLibsCS);
	for (int i = 0; i < mLibs.size(); i++)
	{
		if (name == mLibs[i].GetName())
			return i;
	}
	return -1;
}

const Playlist * FileManager::GetPlaylist(const wxString & libName, 
		const wxString & playlistName) const
{
	int ind = FindLib(libName); //critical section inside
	assert(ind > -1);
	wxCriticalSectionLocker l(mLibsCS);
	return mLibs[ind].GetPlaylist(playlistName);
}

void FileManager::Search(wxString libName, wxString playlistName)
{
	mCurrLib = libName;
	mCurrPlaylist = playlistName;	
	mpThread = new SearcherThread(this, mHandlerFrame);
   	if ( mpThread->Run() != wxTHREAD_NO_ERROR )
	{
		delete mpThread;
		mpThread = NULL;
		throw MyException("Failed to run searcher thread!", 
				MyException::FATAL_ERROR);
	}
}

int FileManager::FromLib(const wxString & filename)
{
	wxCriticalSectionLocker l(mLibsCS);
	wxString ext = GetExtension(filename);
	for (int i = 0; i < mLibs.size(); i++)
	{
		if (mLibs[i].IsRightExtension(ext))
		{
			return i;
		}
	}
	return -1;
}

wxString GetExtension(const wxString & path)
{
	return path.Mid(path.find_last_of('.')+1);
}

wxThread::ExitCode FileManager::SearcherThread::Entry()
{
	mState = WORKING;
	StartAnalyserThread();
	int r;
	try
	{
#ifdef __LINUX__
		mDir.Open("/home/");
		r = mDir.Traverse(*this, wxEmptyString, wxDIR_DIRS | wxDIR_FILES | wxDIR_NO_FOLLOW);
		if (r == -1)
			throw MyException("wxDir::Traverse failed", 
				MyException::FATAL_ERROR);
#ifdef NDEBUG
		if (mStopped)
		{
			StopAnalyserThread();
			return (wxThread::ExitCode)0;
		}
		mDir.Open("/media/");
		r = mDir.Traverse(*this, wxEmptyString, wxDIR_DIRS | wxDIR_FILES |
				wxDIR_NO_FOLLOW);
		if (r == -1)
			throw MyException("wxDir::Traverse failed", 
				MyException::FATAL_ERROR);
		if (mStopped)
		{
			StopAnalyserThread();
			return (wxThread::ExitCode)0;
		}
#endif//NDEBUG

#endif //__LINUX__

#ifdef __WINDOWS__
		//it will find folders on windows without permisions to see files in them
		//so we supress errors
		wxLogNull logNull;
		//first search C:\Users, because media is likely to be there
		if (!mDir.Open("C:\\Users\\"))
			throw MyException("Failed to open C:\\Users\\",
			MyException::FATAL_ERROR);
		r = mDir.Traverse(*this, wxEmptyString, wxDIR_DIRS | wxDIR_FILES |
			wxDIR_NO_FOLLOW);
		if (r == -1)
			throw MyException("wxDir::Traverse failed", 
				MyException::FATAL_ERROR);

		if (mStopped)
		{
			StopAnalyserThread();
			return (wxThread::ExitCode)0;
		}
		//enumerate drives in windows
		TCHAR drives[512];
		int val = GetLogicalDriveStrings(511, drives);
		if (val == 0)
			throw MyException("GetLogicalDriveStrings() failed",
				MyException::NOT_FATAL);
		else
		{
			int len = _tcslen(&drives[0]);
			int n = 0;
			while (len > 0)
			{
				//if can't open don't try (may be empty disk drive or something)
				if (!mDir.Open(&drives[n]))
				{
					n += len + 1;
					len = _tcslen(&drives[n]);
					continue;
				}

				if (mStopped)
				{
					StopAnalyserThread();
					return (wxThread::ExitCode)0;
				}
				r = mDir.Traverse(*this, wxEmptyString, wxDIR_DIRS | wxDIR_FILES
					| wxDIR_NO_FOLLOW);
				if (r == -1)
					throw MyException("wxDir::Traverse failed", 
						MyException::FATAL_ERROR);

				n += len + 1;
				len = 0;
				len = _tcslen(&drives[n]);
			}
		}

#endif //__WINDOWS__

	}
	catch (const MyException & exc)
	{
		StopAnalyserThread();
		throw exc;
	}
	if (!mStopped)
	{
		wxQueueEvent(mHandlerFrame,
			new wxThreadEvent(wxEVT_THREAD, EVT_SEARCHER_UPDATING));
	}
	SignalAndWait();
	if (!mStopped)
	{
		wxQueueEvent(mHandlerFrame,
			new wxThreadEvent(wxEVT_THREAD, EVT_SEARCHER_COMPLETE));
	}

	return (wxThread::ExitCode)0; // success
}

wxDirTraverseResult 
	FileManager::SearcherThread::OnFile(const wxString& filename)
{
	if (!TestDestroy())
	{
		//TODO add function - FromPlaylist
		int ind = mFManager->FromLib(filename);
		wxString type;
		if (ind > -1)
		{
			type = mFManager->mLibs[ind].GetName();
			FileEvent ev(type, filename, ind);
			mQueue.Post(ev);
		}
		return wxDIR_CONTINUE;
	}
	else
	{
		mStopped = true;	
		return wxDIR_STOP;
	}
}

wxDirTraverseResult 
	FileManager::SearcherThread::OnDir(const wxString& dirname)
{
	if (!TestDestroy())
	{
#ifndef NDEBUG
		wxMessageOutputDebug().Printf(dirname);
#endif //NDEBUG
		if (dirname.Matches("*Program Files") || 
				dirname.Matches("*Program Files (x86)") 
				|| dirname.Matches("*Windows") || 
				dirname.Matches("*AppData") || 
				dirname.Matches("*ProgramData") || 
				dirname.Matches("C:\\Users"))
			return wxDIR_IGNORE;
		return wxDIR_CONTINUE;
	}
	else
	{
		mStopped = true;
		return wxDIR_STOP;
	}
}

FileManager::SearcherThread::~SearcherThread()
{
	 wxCriticalSectionLocker enter(mFManager->mThreadCS);
	// the thread is being destroyed; make sure not to leave dangling pointers around
	mFManager->mpThread = NULL;
}

void FileManager::StopSearch()
{
	{
		wxCriticalSectionLocker enter(mThreadCS);
		if (mpThread) // does the thread still exist?
		{
			wxMessageOutputDebug().Printf("FileManager: deleting thread");
			if (mpThread->Delete() != wxTHREAD_NO_ERROR )
				wxLogError("Can't delete searcher thread!");
		}
	} // exit from the critical section to give the thread
 	// the possibility to enter its destructor
	// (which is guarded with mpThreadCS critical section!)
	while (1)
	{
		{ // was the ~MyThread() function executed?
			wxCriticalSectionLocker enter(mThreadCS);
			if (!mpThread) break;
		}
		// wait for thread completion
		wxThread::This()->Sleep(1);
	}
}

FileManager::SearcherThread::SearcherThread(
	FileManager * fMan, PlayerFrame *handler)
	: wxThread(wxTHREAD_DETACHED), wxDirTraverser(), mHandlerFrame(handler),
	mFManager(fMan), mStopped(false), mThread(&mQueue, mFManager, &mState,
		&mStateCS, mHandlerFrame)
{ 
}

const File * FileManager::GetFile(const wxString & libName, 
		const wxString & plName, const int & id) const
{
	const Playlist * pl = GetPlaylist(libName, plName);
	const File * file = pl->GetFile(id);
	return file;
}

FileManager::~FileManager()
{
	if (IsSearching())
		StopSearch();
	wxCriticalSectionLocker l(mFilesCS);
	for (int i = 0; i < mFiles.size(); i++)
		wxDELETE(mFiles[i]);
}

wxVector<long> FileManager::FindFilesInPlaylist(const wxString & libName,
		const wxString & plName, const wxString & mask) const
{
	const Playlist * pl = GetPlaylist(libName, plName);
	return pl->FindFiles(mask);
}

wxVector<long> Playlist::FindFiles(const wxString & mask) const
{
	wxVector<long> matchingInd;
	wxCriticalSectionLocker l(mFilesCS);
	for (int i = 0; i < mFiles.size(); i++)
	{
		wxString fName = mFiles[i]->GetName();
		wxString temp = mask;
		if ( fName.Lower().Matches(temp.Lower()))
			matchingInd.push_back(i);		
	}
	return matchingInd;
}

bool FileManager::IsFound(const wxFileName & filename) const
{
	wxCriticalSectionLocker l(mFilesCS);
	for (int i = 0; i < mFiles.size(); i++)
	{
		if (mFiles[i]->GetFullPath() == filename.GetFullPath())
			return true;
	}
	return false;
}

void FileManager::SearcherThread::StartAnalyserThread()
{
	if ( mThread.Run() != wxTHREAD_NO_ERROR )
	{
		throw MyException("Failed to run searcher thread!", 
				MyException::FATAL_ERROR);
	}
}

void FileManager::SearcherThread::StopAnalyserThread()
{
	assert(mThread.IsAlive());
	{
		wxCriticalSectionLocker enter(mStateCS);
		mState = TERMINATE;
	}
	mQueue.Post(FileEvent()); //wake up thread (might be waiting for event)
	mThread.Wait(); //now wait for thread quiting
}

FileManager::AnalyserThread::AnalyserThread(wxMessageQueue<FileEvent> * msgQueue, 
	FileManager * fMan, State * state, wxCriticalSection * stateCS, PlayerFrame * frame)
	 : wxThread(wxTHREAD_JOINABLE),
	 mQueue(msgQueue), mFManager(fMan), mState(state), mStateCS(stateCS),
	mHandlerFrame(frame)
{
}

wxThread::ExitCode FileManager::AnalyserThread::Entry()
{
	FileEvent ev;
	File * file;
	while (!TestDestroy() || *mState != TERMINATE)
	{
		if (mQueue->Receive(ev) != wxMSGQUEUE_NO_ERROR)
			break;
		if (TestDestroy() || *mState == TERMINATE) //termination can happen while waiting for event
			break;
		if (ev.mType == "done")	//this is signal that there is no files left
		{
			Signal(DONE);
			break;
		}
		assert(ev.mInd != -1);
		try
		{
			if (ev.mType == "Music")
				file = new MusicFile(ev.mPath);
			else if (ev.mType == "Video")
				file = new VideoFile(ev.mPath);
			else //TODO: create other file types
				continue;

		}
		catch (MyException & exc)
		{
			//if failed to initialize object don't put it in the list
			if (exc.type == MyException::NOT_FATAL)
				continue;
			else
				throw;
		}

		//test again, cause creating mediafile can take time
		if (TestDestroy() || *mState == TERMINATE)
		{
			wxDELETE(file);
			break;
		}
		wxCriticalSectionLocker l(mFManager->mFilesCS);
		mFManager->mFiles.push_back(file);
		mFManager->mLibs[ev.mInd].AddFile(mFManager->mFiles.back());
		wxThreadEvent * ev = new wxThreadEvent(wxEVT_THREAD, EVT_SEARCHER_UPDATE);
		ev->SetString("all");
		ev->SetPayload<const File*>(file);
		if (mHandlerFrame != nullptr)
			wxQueueEvent(mHandlerFrame, ev);
		file = nullptr;
	}
	return (wxThread::ExitCode)0; // success
}

void FileManager::SearcherThread::SignalAndWait()
{
	FileEvent ev("done", "", -1);
	mQueue.Post(ev);
	while (1)
	{
		Sleep(10);
		if (mState == DONE)
			break;
		else if (TestDestroy())
		{
			StopAnalyserThread();
			break;
		}
	}
}
