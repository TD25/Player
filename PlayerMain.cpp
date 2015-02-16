// PlayerMain.cpp
// Tadas V.	2014.11.30

#include "wx/thread.h"
#include <assert.h>
#include "Common.h"
#include "PlayerFrame.h"
#include "FileManager.h"
#include "wx/log.h"
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/mediactrl.h"
#include "checkedlistctrl.h"
#include "wx/dcclient.h"
#include "File.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif


// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "checkedlistctrl.h"

//#include "wx/mediactrl.h"

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
//#ifndef wxHAS_IMAGES_IN_RESOURCES
//    #include "../sample.xpm"
//#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------
//function which called on new file and forward it to PlayerApp


// Define a new application type, each program should derive a class from wxApp
class PlayerApp : public wxApp
{
private:
	FileManager * mFManager;
	PlayerFrame * mFrame;
	wxMediaCtrl * mMediaCtrl;
	const File * mPlayedFile;
	wxTimer * mSliderTimer;
	wxTimer * mSecondTimer;
	bool mInitialised;
public:
    // override base class virtuals
	PlayerApp() : mMediaCtrl(nullptr), wxApp(), mInitialised(false) {}
    // ----------------------------
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
	//functions to handle exceptions
	virtual bool OnExceptionInMainLoop();
	virtual void OnUnhandledException();
	void AddLib(wxString name, wxString extensions[], int nExt, 
			wxString columns[] = NULL, int n = 0);
	int OnExit()
	{
		mFManager->StopSearch();
		StopTimer();
		delete mFManager;
		return 0;
	}
	void Play(const wxString & libName, const wxString & plName, 
			const long id); 
	void Pause();
	void MediaLoaded();
	void PlayNew(const File * file);
	bool IsPlaying()
	{
		if (mMediaCtrl->GetState() == wxMEDIASTATE_PLAYING)
			return true;
		else
			return false;
	}
	bool IsPaused()
	{
		if (mMediaCtrl->GetState() == wxMEDIASTATE_PAUSED)
			return true;
		else
			return false;
	}
	void StopTimer()
	{
		mSliderTimer->Stop();
		mSecondTimer->Stop();
	}
	void SetVolume(double val)
	{
		mMediaCtrl->SetVolume(val);
	}
	void Seek(double part)
	{
//		bool playing = (mMediaCtrl->GetState() == wxMEDIASTATE_PLAYING);
		const MediaFile * medFile = dynamic_cast<const MediaFile*>(mPlayedFile);
		if (medFile == nullptr)
			return;
		wxFileOffset length = medFile->GetLength();
		wxFileOffset off = length * part;
		if (off == 0)
			off = 1;
		if (length <= off)
			off = length - 5;	
		mMediaCtrl->Seek(off);
	}
	wxVector<long> FindFileIds(const wxString & libName, 
			const wxString & plName, const wxString & str)
	{
		return mFManager->FindFilesInPlaylist(libName, plName, str);
	}
	wxString GetFileName(const wxString & libName, 
			const wxString & plName, const int & id)
	{
		const wxFileName * file = mFManager->GetFile(libName, plName, id);
		return file->GetName();
	}
	const File * GetFile(const wxString & libName, 
			const wxString & plName, const int & id)
	{
		return mFManager->GetFile(libName, plName, id);
	}
	void OnSearchCommand();
	void OnStopSearchCommand();
   	wxFileOffset Tell();
	void SetVideoMode(bool show);
	void SetVideoSize(const wxPoint & pos, const wxSize & size);
	bool IsInitialised() const {return mInitialised;}
	void StartTimers(int sliderTimerVal = -1); //if -1 previous
};
// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(PlayerFrame, wxFrame)
	EVT_CLOSE(PlayerFrame::OnClose)
    EVT_MENU(QUIT,  PlayerFrame::OnQuit)
    EVT_MENU(ABOUT, PlayerFrame::OnAbout)
	EVT_MENU(SEARCH, PlayerFrame::OnSearchDrive)
	EVT_MENU(STOP_SEARCH, PlayerFrame::OnStopSearch)
   	EVT_COMMAND(wxID_ANY, EVT_SEARCHER_UPDATE, PlayerFrame::OnNewItem)
	EVT_COMMAND(wxID_ANY, EVT_SEARCHER_COMPLETE, 
			PlayerFrame::OnSearchCompletion)
	EVT_BUTTON(CTRL_VOL_BUTTON, PlayerFrame::OnVolButton)
	EVT_MEDIA_LOADED(MEDIA_CTRL, PlayerFrame::OnMediaLoaded)
	EVT_MEDIA_STOP(MEDIA_CTRL, PlayerFrame::OnMediaStop)
	EVT_MEDIA_FINISHED(MEDIA_CTRL, PlayerFrame::OnMediaFinish)
	EVT_MEDIA_PLAY(MEDIA_CTRL, PlayerFrame::OnMediaPlay)
	EVT_BUTTON(CTRL_PLAY, PlayerFrame::OnPlayBt)
	EVT_LIST_ITEM_ACTIVATED(CTRL_LIST, PlayerFrame::OnListActivated)
	EVT_LIST_ITEM_CHECKED(CTRL_LIST, PlayerFrame::OnChecked)
	EVT_LIST_ITEM_UNCHECKED(CTRL_LIST, PlayerFrame::OnUnchecked)
	EVT_LIST_ITEM_SELECTED(CTRL_LIST, PlayerFrame::OnSelected)
	EVT_LIST_ITEM_DESELECTED(CTRL_LIST, PlayerFrame::OnDeselected)
	EVT_TIMER(TIMER_SLIDER, PlayerFrame::OnSliderTimer)	
	EVT_TIMER(TIMER_TIME, PlayerFrame::OnSecondTimer)
	EVT_SLIDER(CTRL_VOL_SLIDER, PlayerFrame::OnVolSlider)
	EVT_SCROLL_CHANGED(PlayerFrame::OnSlider)
	EVT_BUTTON(CTRL_FORWARD, PlayerFrame::OnForward)
	EVT_BUTTON(CTRL_REVERSE, PlayerFrame::OnReverse)
	EVT_TEXT(CTRL_SEARCH, PlayerFrame::OnSearch)
	EVT_SIZE(PlayerFrame::OnSize)
wxEND_EVENT_TABLE()

//define events sent by worker threads
wxDEFINE_EVENT(EVT_SEARCHER_COMPLETE, wxThreadEvent);
wxDEFINE_EVENT(EVT_SEARCHER_UPDATE, ListUpdateEv);

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. PlayerApp and
// not wxApp)
IMPLEMENT_APP(PlayerApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool PlayerApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

	//init MediaInfo lib
	//MediaInfo::Options("Info_Version", "**VERSION**;**APP_NAME**;**APP_VERSION**")
	//MediaInfoLib::MediaInfo::Option_Static(
	//		L"Info_Version", L"0.7.7.1;Player;0.4.2");
    // create the main application window
	int xScreen = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
	int yScreen = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
	wxSize size = wxSize(xScreen*2.5/5, yScreen *2.5/4);
	wxPoint pos = wxPoint(xScreen/4.5,
		   	yScreen/4.5);
    mFrame = new PlayerFrame("Player", pos, size, this);

	mSliderTimer = new wxTimer(mFrame, TIMER_SLIDER);
	mSecondTimer = new wxTimer(mFrame, TIMER_TIME);
	mMediaCtrl = new wxMediaCtrl;
#ifdef __WINDOWS__ //because default doesn't work for some reason
	if (!mMediaCtrl->Create(mFrame, MEDIA_CTRL, wxEmptyString, 
		wxDefaultPosition, wxDefaultSize, 0, wxMEDIABACKEND_WMP10))
		throw MyException("Can't create wxMediaCtrl", 
				MyException::FATAL_ERROR);
#else 
	if (!mMediaCtrl->Create(mFrame, MEDIA_CTRL))
		throw MyException("Can't create wxMediaCtrl", 
				MyException::FATAL_ERROR);
#endif //__WINDOWS__	
	mMediaCtrl->Hide();
    	mFrame->Show(true);
	mFManager = new FileManager(mFrame);
	//add libs and start searching
	wxString extensions[4] = {"mp3", "flac", "aac", "wma"};
	AddLib("Music", extensions, 4);
	
	wxString vidExtensions[5] = {"mp4", "avi", "flv", "wmv", "mov"};
	AddLib("Video", vidExtensions, 5);
//	//TODO: add more image formats
	//wxString picExtensions[5] = {"bmp", "gif", "jpeg", "png", "tif"};
	//AddLib("Pictures", picExtensions, 5);
	mFrame->MakeColumns("Music");

	OnSearchCommand();

	mInitialised = true;
//	set library to which's new files we are notified as they are found
//	const Playlist * pl = mFManager.GetPlaylist("Music", "all");

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------

bool PlayerApp::OnExceptionInMainLoop()
{
	wxString error;
	try { throw; }

	catch (const MyException & e)
	{
		wxMessageBox( e.what(), wxT("My Exception"), wxICON_ERROR);
		if (e.type == MyException::FATAL_ERROR)
			return false;
		else
			return true;
	}
	catch (const std::exception& e) 
	{
		wxMessageBox( e.what(), wxT("runtime_error"), wxICON_ERROR);
		// Exit the main loop and thus terminate the program.
		return false;
	}
}

void PlayerApp::OnUnhandledException()
{
	 OnExceptionInMainLoop();
	 mFManager->StopSearch();
}

void PlayerApp::AddLib(wxString name, wxString extensions[], int extN, 
		wxString columns[], int n)
{
	mFManager->AddLib(MediaLibrary(name, extensions, extN));
	mFrame->AddLib(name);		
}

void PlayerApp::Play(const wxString & libName, const wxString & plName,
		const long id)
{

	const File * file = mFManager->GetFile(libName, plName, id);	
	if (mMediaCtrl->GetState() == wxMEDIASTATE_PLAYING)
	{
		if (file == mPlayedFile)
			throw MyException("Tried to play, but already playing",
				MyException::NOT_FATAL);
		else
			PlayNew(file);
	}
	else if (mMediaCtrl->GetState() == wxMEDIASTATE_PAUSED)
	{
		if (mPlayedFile == file)
		{
			mMediaCtrl->Play();
			mSliderTimer->Start(-1);
			mSecondTimer->Start(-1);
			mFrame->ShiftPlayBt(true);
		}
		else
			PlayNew(file);
	}
	else
		PlayNew(file);

}

void PlayerApp::MediaLoaded()
{
	if (!mMediaCtrl->Play())	
		throw MyException("Failed to play file", MyException::NOT_FATAL);
	const MediaFile * medFile = dynamic_cast<const MediaFile*>(mPlayedFile);
	if (medFile->GetType() == "Video")
		SetVideoMode(true);
	wxFileOffset length = 0;
	if (medFile != nullptr)
		length = medFile->GetLength();
	if (mSliderTimer->IsRunning())
	{
		mSliderTimer->Stop();
		mSecondTimer->Stop();
	}
	int d = length / SLIDER_MAX_VAL;
	if (length < 2000)
		mSliderTimer->StartOnce(length / SLIDER_MAX_VAL);
	else
		mSliderTimer->Start(length / SLIDER_MAX_VAL);
	mSecondTimer->Start(1000);
	mFrame->ShiftPlayBt(true);
}

void PlayerFrame::OnMediaLoaded(wxMediaEvent& ev)
{
	mSecondsPlaying = 0;
	mTimePanel->ChangeText("0.0/0.0");
	double selection = mVolSlider->GetValue();
	mApp->SetVolume(selection / SLIDER_MAX_VAL);
	mApp->MediaLoaded();
	DrawName();
	mList->Check(mCurrItemInList, false);
	const File * file = mApp->GetFile(mLibNames[mActiveLib], 
			mPlaylistNames[mActiveLib][mActivePlaylist], mCurrItemId);
	const MediaFile * mdFile = dynamic_cast<const MediaFile*>(file);
	if (mdFile == nullptr)
		throw MyException("PlayerFrame::OnMediaLoaded(): failed dynamic_cast to media file", MyException::FATAL_ERROR);
	mCurrLengthStr = mdFile->GetLengthStr();
//	for (int i = 1; i < mSelectedItems.size(); i++)
//		Deselect(mSelectedItems[i]);
//	Select(GetCurrSelection());
}


void PlayerFrame::OnPlayBt(wxCommandEvent& ev)
{
	if (mPlayBt->GetLabel() == ">")
	{
		if (!mApp->IsPaused())
			OnPlay(true);
		else
			OnPlay(false);
	}
	else
		mApp->Pause();
}

void PlayerApp::PlayNew(const File * file)
{
	wxString path = file->GetFullPath();
	mFrame->ResetSlider();
	if (!mMediaCtrl->Load(path))
		throw MyException("Failed to load file in PlayerApp::PlayNew()",
				MyException::NOT_FATAL);
	mPlayedFile = file;
}

void PlayerFrame::OnListActivated(wxListEvent& ev)
{
	OnPlay(true);
}

void PlayerApp::Pause()
{

	if (mMediaCtrl->GetState() == wxMEDIASTATE_PLAYING)
	{
		mMediaCtrl->Pause();
		StopTimer();
		mFrame->ShiftPlayBt(false);

	}
	else
		throw MyException("Tried to pause altough is not playing",
				MyException::NOT_FATAL);
}

void PlayerFrame::OnMediaFinish(wxMediaEvent& ev)
{
	wxString str;
	str.Printf(wxT("%s/%s"), mCurrLengthStr, mCurrLengthStr);
	mTimePanel->ChangeText(str);
	mSlider->SetValue(SLIDER_MAX_VAL);
	mApp->StopTimer();

	if (mCheckedItems.size() > 0)
	{
		mCurrItemInList = mCheckedItems[0];
		mCurrItemId = mListMap[mCurrItemInList];
		int i = Find(mSelectedItems, mCurrItemInList);
		if (i > -1)
			mSelectedItems.erase(mSelectedItems.begin()+i);
		mApp->Play(mLibNames[mActiveLib], 
				mPlaylistNames[mActiveLib][mActivePlaylist], mCurrItemId);
//		mList->Check(mCurrItemInList, false);
		mCheckedItems.erase(mCheckedItems.begin());
	}	
	else
	{
		ShiftPlayBt(false);
	}
}

void PlayerFrame::OnPlay(bool lookForNew)
{
	if (lookForNew)
	{
		try
		{
			mCurrItemInList = GetCurrSelectionInList();
			mCurrItemId = mListMap[mCurrItemInList];
		}
		catch (MyException & exc)
		{
			if (mCurrItemId == -1 || mCurrItemInList == -1)
				throw;
		}
		mApp->Play(mLibNames[mActiveLib], 
				mPlaylistNames[mActiveLib][mActivePlaylist], mCurrItemId);
//		mList->Check(mCurrItemInList, false);
		DeleteCurrSelection();
		for (int i = 0; i < mSelectedItems.size(); i++)
		{
			mCheckedItems.push_back(mSelectedItems[i]);
			mList->Check(mSelectedItems[i], true);
		}
		mSelectedItems.clear();
	}
	else
		mApp->Play(mLibNames[mActiveLib], 
				mPlaylistNames[mActiveLib][mActivePlaylist], mCurrItemId);
}

wxFileOffset PlayerApp::Tell()
{
	if (mMediaCtrl->GetState() != wxMEDIASTATE_PLAYING)
		throw MyException("Not playing: PlayerApp::Tell()", 
				MyException::NOT_FATAL);
	return mMediaCtrl->Tell();
}

void PlayerFrame::OnSecondTimer(wxTimerEvent& ev)
{
	static int fails = 0;
	wxFileOffset off;
	try
	{
		off = mApp->Tell();
	}
	catch (MyException & exc)
	{
		fails++;
		if (fails == 5)
		{
			throw exc;
		}
	}
	int seconds = off / 1000;
	int secondsPl = seconds % 60;
	int minutesPl = (seconds-secondsPl) / 60;
	wxString str = FormatTime(minutesPl, secondsPl);
	str.Printf(wxT("%s/%s"), str, mCurrLengthStr);
	mTimePanel->ChangeText(str);
}

void PlayerFrame::OnVolSlider(wxCommandEvent& ev)
{
	double selection = mVolSlider->GetValue();
	mApp->SetVolume(selection / SLIDER_MAX_VAL);
}

void PlayerFrame::OnSlider(wxScrollEvent& ev)
{
	if (ev.GetId() == CTRL_SLIDER)
	{
		double part = (double)mSlider->GetValue() / SLIDER_MAX_VAL; 
		mApp->Seek(part);
	}
}

void PlayerFrame::OnForward(wxCommandEvent& ev)
{
	if (mCheckedItems.size() > 0)
	{
		wxMediaEvent ev;
		mApp->Seek(1);
		OnMediaFinish(ev);
	}
	else
	{
		if (mCurrItemInList < mList->GetItemCount() - 1)
		{
			mCurrItemInList++;
			mCurrItemId = mListMap[mCurrItemInList];
			mCheckedItems.push_back(mCurrItemInList);
			mApp->Seek(1);
			OnPlay(true);
		}
	}
}

void PlayerFrame::OnReverse(wxCommandEvent& ev)
{
	if (mCurrItemInList > 0)
	{
		mCurrItemInList--;
		mCurrItemId = mListMap[mCurrItemInList];
		mCheckedItems.insert(mCheckedItems.begin(), mCurrItemInList);
		wxMediaEvent ev;
		OnMediaFinish(ev);
	}
}

void PlayerFrame::OnSearch(wxCommandEvent& ev)
{
//	ev.Skip();
	wxString str = mSearchBox->GetValue();
	wxString mask;
	mask.Printf("*%s*", str);
//	if (str.size() < 5)
//		return;
	int start = -1;
	int r = 0;

	//TODO handle this part more efficiently by getting files
	//	with names and id's, so that you don't need to call 
	//	mApp->GetFileNames()
	wxVector<long> indexes = mApp->FindFileIds(mLibNames[mActiveLib],
			mPlaylistNames[mActiveLib][mActivePlaylist], mask);

	if (indexes.size() > 0)
	{
		mCheckedItems.clear();
		mSelectedItems.clear();
		mListMap = indexes;
		wxVector<const File *> newItems;
		for (int i = 0; i < indexes.size(); i++)
		{
			newItems.push_back(mApp->GetFile(mLibNames[mActiveLib],
						mPlaylistNames[mActiveLib][mActivePlaylist], 
						indexes[i]));
		}
		mList->DeleteAllItems();
		for (int i = 0; i < newItems.size(); i++)
		{
			AddListItem(newItems[i]);
		}
	}

}

wxString FormatTime(int minutes, int seconds)
{
	wxString str;
	if (seconds < 10)
		str.Printf("%d:0%d", minutes, seconds);
	else
		str.Printf("%d:%d", minutes, seconds);
	return str;
}

const File * PlayerFrame::GetCurrFile() const
{
	if (mActiveLib < 0 || mActivePlaylist < 0 || mCurrItemId < 0)
		throw MyException("PlayerFrame::GetCurrFile(): mCurrItem not set",
				MyException::FATAL_ERROR);
	return mApp->GetFile(mLibNames[mActiveLib], 
		mPlaylistNames[mActiveLib][mActivePlaylist], mCurrItemId);	
}

void PlayerFrame::OnSearchDrive(wxCommandEvent& ev)
{
	mApp->OnSearchCommand();
}

void PlayerApp::OnSearchCommand()
{
	if (mFManager->GetLibCount() <= 0)
		throw MyException("PlayerApp::OnSearchCommand(): searching but no libraries set", MyException::NOT_FATAL);
	if (mFManager->IsSearching())
		throw MyException("Searching already!", MyException::NOT_FATAL);
	mFManager->Search();
	mFrame->mFileMenu->Enable(SEARCH, false);
	mFrame->mFileMenu->Enable(STOP_SEARCH, true);
	wxLogStatus(mFrame, "Searching...");
}

void PlayerApp::OnStopSearchCommand()
{
	if (!mFManager->IsSearching())
		throw MyException("Wasn't searching", MyException::NOT_FATAL);
	mFManager->StopSearch();
	mFrame->mFileMenu->Enable(SEARCH, true);
	mFrame->mFileMenu->Enable(STOP_SEARCH, false);
	wxLogStatus(mFrame, "Searching stopped");
}

void PlayerFrame::OnStopSearch(wxCommandEvent& ev)
{
	mApp->OnStopSearchCommand();
}

void PlayerApp::SetVideoMode(bool full)
{
	static wxPoint prevPos;
	static wxSize prevSize;
	if (full)
	{
		prevPos = mFrame->GetPosition();
		prevSize = mFrame->GetSize();
		mFrame->Maximize(true);
		mMediaCtrl->Show(true);
	}
	else
	{
		mFrame->SetPosition(prevPos);
		mFrame->SetSize(prevSize);
		mMediaCtrl->Hide();
	}
}

void PlayerApp::SetVideoSize(const wxPoint & pos, const wxSize & size)
{
	if (mMediaCtrl == nullptr)
		return;
	mMediaCtrl->SetPosition(pos);
	mMediaCtrl->SetSize(size);
}

void PlayerFrame::OnSize(wxSizeEvent& ev)
{
	ev.Skip();
	if (!mApp->IsInitialised())
		return;
	wxSize size(ev.GetSize().x, 
			ev.GetSize().y-mMediaCtrlsPanel->GetSize().y);
	wxPoint pos(0, mMediaCtrlsPanel->GetSize().y);
	mApp->SetVideoSize(pos, size);
}

void PlayerFrame::OnMediaStop(wxMediaEvent& ev)
{
	ShiftPlayBt(false);
	mApp->StopTimer();
}

void PlayerFrame::OnMediaPlay(wxMediaEvent& ev)
{
	ShiftPlayBt(true);
	mApp->StartTimers();
}

void PlayerApp::StartTimers(int sliderTimerVal)
{
	mSecondTimer->Start(1000);
	mSliderTimer->Start(sliderTimerVal);
}

void PlayerFrame::OnClose(wxCloseEvent & evt)
{
	mApp->StopTimer();
	Destroy();
}
