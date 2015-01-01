#include "FileManager.h"
#include "wx/event.h"
#include "PlayerFrame.h"
#include "wx/log.h"
#ifdef __WINDOWS__
	#include <windows.h>
	#include <string>
#endif

ListUpdateEv::ListUpdateEv(const File * file) : 
	wxCommandEvent(EVT_SEARCHER_UPDATE), mpFile(file)	
	{}

void FileManager::AddLib(MediaLibrary lib)
{
	if (FindLib(lib.GetName()) != -1)
		throw MyException("There already exists library with this name",
				MyException::NOT_FATAL);
	mLibs.push_back(lib);
}

void FileManager::AddLib(const wxString & name)
{
	if (FindLib(name) != -1)
		throw MyException("There already exist library with this name",
				MyException::NOT_FATAL);
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
	int ind = FindLib(libName);
	assert(ind > -1);
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

int FileManager::FromLib(const wxFileName & file)
{
	wxString ext = file.GetExt();
	for (int i = 0; i < mLibs.size(); i++)
	{
		if (mLibs[i].IsRightExtension(ext))
		{
			return i;
		}
	}
	return -1;
}

wxThread::ExitCode FileManager::SearcherThread::Entry()
{

	int r;
#ifdef __LINUX__
	mDir.Open("/home/");
	r = mDir.Traverse(*this, wxEmptyString, wxDIR_DIRS | wxDIR_FILES | wxDIR_NO_FOLLOW);
	if (r == -1)
		throw MyException("wxDir::Traverse failed", 
			MyException::FATAL_ERROR);
#ifdef NDEBUG
	mDir.Open("/media/");
	r = mDir.Traverse(*this, wxEmptyString, wxDIR_DIRS | wxDIR_FILES |
			wxDIR_NO_FOLLOW);
	if (r == -1)
		throw MyException("wxDir::Traverse failed", 
			MyException::FATAL_ERROR);
#endif//NDEBUG

#endif //__LINUX__

#ifdef __WINDOWS__
	//it will find folders on windows without permisions to see files in them
		//so we supress errors
	wxLog::SetLogLevel(wxLOG_FatalError);
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
				n += len+1;
				len = _tcslen(&drives[n]);
				continue;
			}
			r = mDir.Traverse(*this, wxEmptyString, wxDIR_DIRS | wxDIR_FILES |	wxDIR_NO_FOLLOW);
			n += len+1;
			len = 0;
			len = _tcslen(&drives[n]);
		}
	}
	wxLog::SetLogLevel(wxLOG_Max) ;

#endif //__WINDOWS__

	if (!mStopped)
	{
		wxQueueEvent(mHandlerFrame, 
			new wxThreadEvent(EVT_SEARCHER_COMPLETE));
	}
	

	 return (wxThread::ExitCode)0; // success
}

wxDirTraverseResult 
	FileManager::SearcherThread::OnFile(const wxString& filename)
{
	if (!TestDestroy())
	{
		wxCriticalSectionLocker enter(mFManagerCS);
		wxFileName name(filename);
		if (mFManager->IsFound(name))
			return wxDIR_CONTINUE;
		//TODO add function - FromPlaylist
		int ind = mFManager->FromLib(name);
		if (ind > -1)
		{
			wxString type = mFManager->mLibs[ind].GetName();
			File * file;
			if (type == "Music")
			{
				try
				{
					file = new MusicFile(name);
				}
				catch (MyException & exc)
				{
					//if failed to initialize object don't put it in the list
					if (exc.type == MyException::NOT_FATAL)
						return wxDIR_CONTINUE; 
					else
						throw;
				}
			}
			else //TODO: create other file types
				return wxDIR_CONTINUE;
			mFManager->mFiles.push_back(file);
			mFManager->mLibs[ind].AddFile(mFManager->mFiles.back());
			wxQueueEvent(mHandlerFrame, new ListUpdateEv(file));
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
//#ifndef NDEBUG
//		wxMessageOutputDebug().Printf(dirname);
//#endif //NDEBUG
		if (dirname.Matches("*Program Files") || 
				dirname.Matches("*Program Files (x86)") 
				|| dirname.Matches("*Windows") || 
				dirname.Matches("*AppData") || 
				dirname.Matches("*ProgramData"))
			return wxDIR_IGNORE;
		return wxDIR_CONTINUE;
	}
	else
		return wxDIR_STOP;
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
		: wxThread(wxTHREAD_DETACHED), wxDirTraverser(), mSearchStage(0),
		mFManager(fMan), mStopped(false)
{ 
	mHandlerFrame = handler;
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
	for (int i = 0; i < mFiles.size(); i++)
		delete mFiles[i];
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
	for (int i = 0; i < mFiles.size(); i++)
	{
		if (mFiles[i]->GetFullPath() == filename.GetFullPath())
			return true;
	}
	return false;
}
