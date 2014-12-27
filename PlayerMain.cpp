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
	wxFileName mPlayedFile;
	wxTimer * mSliderTimer;
	wxTimer * mSecondTimer;
public:
    // override base class virtuals
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
		delete mFManager;
		return 0;
	}
	void Play(const wxString & libName, const wxString & plName, 
			const long id); 
	void Pause();
	void MediaLoaded();
	void PlayNew(const wxFileName * file);
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
		bool playing = (mMediaCtrl->GetState() == wxMEDIASTATE_PLAYING);
		wxFileOffset length = mMediaCtrl->Length();
		wxFileOffset off = length * part;
		if (off == 0)
			off = 1;
		if (length <= off)
			off = length - 5;	
		mMediaCtrl->Seek(off);
	}
	wxFileOffset Tell();
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
   	EVT_COMMAND(wxID_ANY, EVT_SEARCHER_UPDATE, PlayerFrame::OnNewItem)
	EVT_COMMAND(wxID_ANY, EVT_SEARCHER_COMPLETE, 
			PlayerFrame::OnSearchCompletion)
	EVT_BUTTON(CTRL_VOL_BUTTON, PlayerFrame::OnVolButton)
	EVT_BUTTON(CTRL_VOL_SLIDER, PlayerFrame::OnVolume)
	EVT_MEDIA_LOADED(MEDIA_CTRL, PlayerFrame::OnMediaLoaded)
	EVT_MEDIA_STOP(MEDIA_CTRL, PlayerFrame::OnMediaStop)
	EVT_MEDIA_FINISHED(MEDIA_CTRL, PlayerFrame::OnMediaFinish)
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

    // create the main application window
	int xScreen = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
	int yScreen = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
	wxSize size = wxSize(xScreen*2.5/5, yScreen *2.5/4);
	wxPoint pos = wxPoint(xScreen/4.5,
		   	yScreen/4.5);
    mFrame = new PlayerFrame("Player", pos, size, this);

	mSliderTimer = new wxTimer(mFrame, TIMER_SLIDER);
	mSecondTimer = new wxTimer(mFrame, TIMER_TIME);
    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    mFrame->Show(true);
	mMediaCtrl = new wxMediaCtrl;
	mMediaCtrl->SetVolume(0.5);

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

	mFManager = new FileManager(mFrame);
	//add libs and start searching
	wxString extensions[5] = {"mp3", "wav", "flac", "aac", "wma"};
	AddLib("Music", extensions, 5);
	
	wxString vidExtensions[5] = {".mp4", ".avi", ".flv", ".wmv", ".mov"};
	AddLib("Video", vidExtensions, 5);
//	//TODO: add more image formats
	wxString picExtensions[5] = {".bmp", ".gif", ".jpeg", ".png", ".tif"};
	AddLib("Pictures", picExtensions, 5);

	wxLogStatus(mFrame, "Searching...");
	mFManager->Search();

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

	const wxFileName * file = mFManager->GetFile(libName, plName, id);	
	if (mMediaCtrl->GetState() == wxMEDIASTATE_PLAYING)
	{
		if (*file == mPlayedFile)
			throw MyException("Tried to play, but already playing",
				MyException::NOT_FATAL);
		else
			PlayNew(file);
	}
	else if (mMediaCtrl->GetState() == wxMEDIASTATE_PAUSED)
	{
		if (mPlayedFile == *file)
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
	wxFileOffset length = mMediaCtrl->Length();
	mFrame->SetCurrLength(length);
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
	mApp->MediaLoaded();
	DrawName();
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

void PlayerApp::PlayNew(const wxFileName * file)
{
	wxString path = file->GetFullPath();
	mFrame->ResetSlider();
	if (!mMediaCtrl->Load(path))
		throw MyException("Failed to load file in PlayerApp::PlayNew()",
				MyException::NOT_FATAL);
	mPlayedFile = *file;
	mFrame->mCurrName = file->GetName();
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
		mFrame->ShiftPlayBt(false);
		mSliderTimer->Stop();
		mSecondTimer->Stop();
	}
	else
		throw MyException("Tried to pause altough is not playing",
				MyException::NOT_FATAL);
}

void PlayerFrame::OnMediaFinish(wxMediaEvent& ev)
{
	mSlider->SetValue(SLIDER_MAX_VAL);
	mApp->StopTimer();
	if (mCheckedItems.size() > 0)
	{
		mCurrItemId = mCheckedItems[0];
		int i = Find(mSelectedItems, mCurrItemId);
		if (i > -1)
			mSelectedItems.erase(mSelectedItems.begin()+i);
		mApp->Play(mLibNames[mActiveLib], "all", mCurrItemId);
		mList->Check(mCurrItemId, false);
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
			mCurrItemId = GetCurrSelection();
		}
		catch (MyException & exc)
		{
			if (mCurrItemId == -1)
				throw;
		}
		mApp->Play(mLibNames[mActiveLib], "all", mCurrItemId);
		mList->Check(mCurrItemId, false);
		DeleteCurrSelection();
		for (int i = 0; i < mSelectedItems.size(); i++)
		{
			mCheckedItems.push_back(mSelectedItems[i]);
			mList->Check(mSelectedItems[i], true);
		}
		mSelectedItems.clear();
	}
	else
		mApp->Play(mLibNames[mActiveLib], "all", mCurrItemId);
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
	wxFileOffset off = mApp->Tell();
	int seconds = off / 1000;
	int minutesPl = seconds / 60;
	int secondsPl = seconds % 60;
	wxString str;
	str.Printf(wxT("%d:%d/%d:%d"), minutesPl, secondsPl, mCurrLength[0],
			mCurrLength[1]);
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
