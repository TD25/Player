// PlayerMain.cpp
// Tadas V.	2014.11.30

#include "wx/thread.h"
#include <assert.h>
#include "MyException.h"
#include "PlayerFrame.h"
#include "FileManager.h"
#include "wx/log.h"
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/mediactrl.h"

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
			const int id);
	void MediaLoaded();
	void PlayNew(const wxFileName * file);
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

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

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    mFrame->Show(true);
	mMediaCtrl = new wxMediaCtrl;
	if (!mMediaCtrl->Create(mFrame, MEDIA_CTRL))
		throw MyException("Can't create wxMediaCtrl", 
				MyException::FATAL_ERROR);

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
		wxMessageBox(e.what(), wxT("MyException"), wxICON_ERROR);
//		wxLogError("Unexpected exception has occurred: %s", e.what());
		if (e.type == MyException::FATAL_ERROR)
			return false;
		else
			return true;
	}
	catch (const std::exception& e) 
	{
		wxMessageBox( e.what(), wxT("runtime_error"), wxICON_INFORMATION);
//		wxLogError("Unexpected exception has occurred: %s", e.what());
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
		const int id)
{

	const wxFileName * file = mFManager->GetFile(libName, plName, id);	
	mFrame->ShiftPlayBt();
	if (mMediaCtrl->GetState() == wxMEDIASTATE_PLAYING) 
		mMediaCtrl->Pause();
	else if (mMediaCtrl->GetState() == wxMEDIASTATE_PAUSED)
	{
		if (mPlayedFile == *file)
			mMediaCtrl->Play();
		else
			PlayNew(file);
	}
	else
		PlayNew(file);

}

void PlayerApp::MediaLoaded()
{
	mMediaCtrl->Play();	
}

void PlayerFrame::OnMediaLoaded(wxMediaEvent& ev)
{
	mApp->MediaLoaded();
}

void PlayerFrame::OnPlayBt(wxCommandEvent& ev)
{
	mApp->Play(mLibNames[mActiveLib], "all", 1);
}

void PlayerApp::PlayNew(const wxFileName * file)
{
	wxString path = file->GetFullPath();
	mMediaCtrl->Load(path);
	mPlayedFile = *file;
}

