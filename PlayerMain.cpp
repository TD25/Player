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

#include "checkedlistctrl.h"

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
	wxPanel * mLibsPanel;
	wxBoxSizer * mLibsSizer;
	wxCheckedListCtrl * mList;
	wxListBox * mPlayLists;
//	wxBoxSizer * mSizer1;
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
	CTRL_LIST,
	CTRL_PLAYLISTS,
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
	wxSize size = wxSize(xScreen*2.5/5, yScreen *2.5/4);
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


	wxBoxSizer * mainSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * sizer1 = new wxBoxSizer(wxHORIZONTAL);
	//create panel and assign it buttons
	
	SetBackgroundColour(wxTheColourDatabase->Find("DARK GREY"));

	wxPanel * panel1 = new wxPanel(this, wxID_ANY, 
			wxPoint(10, 10), wxSize(size.y, 50));
	panel1->SetBackgroundColour(wxTheColourDatabase->Find("DARK GREY"));

	wxPoint btPos(10, 10);
	wxSize rBtSize(50, 50);
	wxSize btSize = rBtSize;

	wxButton * backBt = new wxButton(panel1, CTRL_REVERSE, "<<",
			btPos, btSize);
	sizer1->AddSpacer(10);
	sizer1->Add(backBt, 1, wxALIGN_CENTER);
	btPos.x += btSize.x;
	wxButton * playBt = new wxButton(panel1, CTRL_PLAY, ">", 
			btPos, btSize);
	sizer1->Add(playBt, 1, wxALIGN_CENTER);
	btPos.x += btSize.x;
	wxButton * forwardBt = new wxButton(panel1, CTRL_FORWARD, ">>",
			btPos, btSize);
	sizer1->Add(forwardBt, 1, wxALIGN_CENTER);
	btPos.x += btSize.x + 20;
	btPos.y += 10;
	sizer1->AddSpacer(10);
	wxSlider * slider = new wxSlider(panel1, CTRL_SLIDER, 0, 0,
		   	maxSliderVal, btPos, wxSize(100, 20));
	sizer1->Add(slider, 4, wxALIGN_CENTER);
	sizer1->AddStretchSpacer(5);
	btPos.x = size.x - 200;
	btSize.y = 25;
	btSize.x = 200;
	wxTextCtrl * searchBox = new wxTextCtrl(panel1, CTRL_SEARCH, "search",
			btPos, btSize);
	sizer1->Add(searchBox, 4, wxALIGN_CENTER);
	
	panel1->SetSizerAndFit(sizer1);

//	mainSizer->Add(sizer1, 0, wxEXPAND);
	mainSizer->AddSpacer(10);
	mainSizer->Add(panel1, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL);
	SetSizer(mainSizer);
	
	//make panel where libs are chosen
	btPos.x = 0;
	btPos.y = rBtSize.y + 20;
	btSize.x = size.x;
	btSize.y = 25;
	mLibsPanel = new wxPanel(this, PANEL_LIBS, btPos, btSize);
	mLibsSizer = new wxBoxSizer(wxHORIZONTAL);
	mLibsPanel->SetBackgroundColour(wxTheColourDatabase->Find("DIM GREY"));
	
	btPos.x = 10;
	btPos.y = 5;
	btSize.x = 15;
	btSize.y = 15;
	AddLib("Music");
	mainSizer->AddSpacer(10);
	mainSizer->Add(mLibsPanel, 0);

	AddLib("Video");
	AddLib("Pictures");

	//make playLists and media list panels
	//media list
	wxBoxSizer * sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxPanel * panel2 = new wxPanel(this, wxID_ANY, wxPoint(0, size.y-40),
			wxSize(size.x, size.y-40));
	panel2->SetBackgroundColour(wxTheColourDatabase->Find("DARK GREY"));

	//playlists
	mPlayLists = new wxListBox(panel2, CTRL_PLAYLISTS, wxPoint(0, 0),
			wxSize(size.x/5, size.y),0, NULL, wxLB_SINGLE | wxLB_NEEDED_SB |
			wxLB_HSCROLL);
	sizer2->Add(mPlayLists, 1, wxEXPAND);
	wxString items[5] = {"item1", "item2", "item3", "item4", "item5"};
	mPlayLists->InsertItems(5, items, 0);


	mList = new wxCheckedListCtrl(panel2, CTRL_LIST, wxPoint(0, 0),
			wxSize(size.x, size.y - 40), wxLC_REPORT | wxLC_HRULES | 
			wxLC_VRULES);

	mList->AppendColumn(wxT("Name"), wxLIST_FORMAT_LEFT, size.x / 4);     
	mList->AppendColumn(wxT("Artist"), wxLIST_FORMAT_LEFT, size.x/4);
	mList->InsertItem(0, "ABC");
	mList->Check(0, FALSE);
	mList->InsertItem(1, "DEF");
	mList->Check(1, TRUE);
	sizer2->AddSpacer(10);
	sizer2->Add(mList, 8, wxEXPAND);
	panel2->SetSizerAndFit(sizer2);
	mainSizer->AddSpacer(10);
	mainSizer->Add(panel2, 10, wxEXPAND);

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
	mLibsSizer->AddSpacer(10);
	mLibsSizer->Add(button, 0, wxSHAPED);
	mLibsPanel->SetSizerAndFit(mLibsSizer);
}
