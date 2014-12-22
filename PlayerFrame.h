#pragma once

#include "wx/wx.h"
#include "checkedlistctrl.h"
#include "wx/popupwin.h"
#include "wx/mediactrl.h"
#include "wx/filename.h"

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
	CTRL_VOL_BUTTON,
	CTRL_VOL_SLIDER,
	MEDIA_CTRL
};

class ListUpdateEv : public wxCommandEvent
{
private:
	wxFileName mFile;
public:
	ListUpdateEv(wxFileName file);
	wxFileName GetFile() {return mFile;}
	 // implement the base class pure virtual
	virtual wxEvent *Clone() const { return new ListUpdateEv(*this); }
};

class PlayerApp;
// Define a new frame type: this is going to be our main frame
class PlayerFrame : public wxFrame
{
public:
    // ctor(s)
    PlayerFrame(const wxString& title, wxPoint & pos, wxSize & size, 
			PlayerApp * pApp);
    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void AddLib(wxString label, wxString columns[] = NULL, int n = 0);
	void AddListItem(wxListItem & listItem);
	void OnNewItem(wxCommandEvent& event);
	void OnSearchCompletion(wxCommandEvent& event) 
	{
		wxLogStatus(this, "Search complete");
	}
	void OnClose(wxCloseEvent &);
	void OnVolButton(wxCommandEvent& ev);
	void OnVolume(wxCommandEvent& ev) {};
	void OnMediaLoaded(wxMediaEvent& ev);
	void OnMediaStop(wxMediaEvent& ev) {}
	void OnMediaFinish(wxMediaEvent& ev) {}
	void OnPlayBt(wxCommandEvent& ev);
	void ShiftPlayBt();	//changes text (bitmap) on play button
	int GetCurrSelection() const;	//determines selection in the list
private:
	wxPanel * mLibsPanel;
	wxBoxSizer * mLibsSizer;
	wxCheckedListCtrl * mList;
	wxListBox * mPlayLists;
	wxSlider * mVolSlider;
	wxPopupTransientWindow * mVolPopup;
	wxSlider * mSlider;
	wxButton * mVolButton;
	wxButton * mPlayBt;
	wxVector<wxString> mLibNames;
	int mActiveLib;
//	wxBoxSizer * mSizer1;
    wxDECLARE_EVENT_TABLE();
	PlayerApp * mApp;
};


