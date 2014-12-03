// PlayerMain.cpp
// Tadas V.	2014.11.30

#include <assert.h>
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//constants
const int maxSliderVal = 100;

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

// Define a new application type, each program should derive a class from wxApp
class PlayerApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Define a new frame type: this is going to be our main frame
class PlayerFrame : public wxFrame
{
public:
    // ctor(s)
    PlayerFrame(const wxString& title, wxPoint & pos, wxSize & size);
    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void AddLib(wxString label);
private:
    // any class wishing to process wxWidgets events must use this macro
	wxPanel * mPanel;	//main panel
	wxPanel * mLibsPanel;
    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    QUIT = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    ABOUT = wxID_ABOUT,
	
	//panel id's
	CTRL_PLAY = wxID_HIGHEST+1,
	CTRL_FORWARD,
	CTRL_REVERSE, 
	CTRL_SEARCH,
	CTRL_SLIDER,
	PANEL_LIBS,
};
// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(PlayerFrame, wxFrame)
    EVT_MENU(QUIT,  PlayerFrame::OnQuit)
    EVT_MENU(ABOUT, PlayerFrame::OnAbout)
wxEND_EVENT_TABLE()

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
	wxSize size = wxSize(xScreen*2.5/4, yScreen *2.5/4);
	wxPoint pos = wxPoint(xScreen/4.5,
		   	yScreen/4.5);
    PlayerFrame *frame = new PlayerFrame("Player", pos, size);

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
PlayerFrame::PlayerFrame(const wxString& title, wxPoint &pos, wxSize &size)
       : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    // set the frame icon
//    SetIcon(wxICON(sample));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(ABOUT, "&About\tF1", "Show about dialog");

    fileMenu->Append(QUIT, "E&xit\tAlt-X", "Quit this program");

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUSB

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText("Welcome to wxWidgets!");
#endif // wxUSE_STATUSBAR

	//create panel and assign it buttons
	mPanel = new wxPanel(this, wxID_ANY, pos, size);
	mPanel->SetBackgroundColour(wxTheColourDatabase->Find("DARK GREY"));

	wxPoint btPos(10, 10);
	wxSize rBtSize(50, 50);
	wxSize btSize = rBtSize;
	wxButton * backBt = new wxButton(mPanel, CTRL_REVERSE, "<<",
			btPos, btSize);
	btPos.x += btSize.x;
	wxButton * playBt = new wxButton(mPanel, CTRL_PLAY, ">", 
			btPos, btSize);
	btPos.x += btSize.x;
	wxButton * forwardBt = new wxButton(mPanel, CTRL_FORWARD, ">>",
			btPos, btSize);
	btPos.x += btSize.x + 20;
	btPos.y += 10;
	wxSlider * slider = new wxSlider(mPanel, CTRL_SLIDER, 0, 0,
		   	maxSliderVal, btPos, wxSize(100, 20));
	btPos.x = size.x - 200;
	btSize.y = 50;
	btSize.x = 200;
	wxTextCtrl * searchBox = new wxTextCtrl(mPanel, CTRL_SEARCH, "search",
			btPos, btSize);
	
	//make panel where libs are chosen
	btPos.x = 0;
	btPos.y = rBtSize.y + 20;
	btSize.x = size.x;
	btSize.y = 25;
	mLibsPanel = new wxPanel(mPanel, PANEL_LIBS, btPos, btSize);
	mLibsPanel->SetBackgroundColour(wxTheColourDatabase->Find("DIM GREY"));
	
	btPos.x = 10;
	btPos.y = 5;
	btSize.x = 15;
	btSize.y = 15;
	AddLib("Music");

}


// event handlers

void PlayerFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void PlayerFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format
                 (
                    "Welcome to %s!\n"
                    "\n"
                    "This is the player\n"
                    "running under %s.",
                    wxVERSION_STRING,
                    wxGetOsDescription()
                 ),
                 "About wxWidgets minimal sample",
                 wxOK | wxICON_INFORMATION,
                 this);
}

void PlayerFrame::AddLib(wxString label)
{
	static int libCount = 0;
	static wxPoint pos(10, 0);
	static wxSize size(25, 25);
	pos.x += libCount * size.x + 5;
	//TODO: add bitmap
	wxButton * button = new wxButton(mLibsPanel, wxID_ANY, label,
			pos, size);
	libCount++;
}

