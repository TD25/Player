#pragma once

#include "wx/wx.h"
#include "checkedlistctrl.h"
#include "File.h"

class ListUpdateEv : public wxCommandEvent
{
private:
	File mFile;
public:
	ListUpdateEv(File file);
	File GetFile() {return mFile;}
	 // implement the base class pure virtual
	virtual wxEvent *Clone() const { return new ListUpdateEv(*this); }
};

class PlayerApp;
// Define a new frame type: this is going to be our main frame
class PlayerFrame : public wxFrame
{
public:
    // ctor(s)
    PlayerFrame(const wxString& title, wxPoint & pos, wxSize & size);
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
private:
    // any class wishing to process wxWidgets events must use this macro
	wxPanel * mLibsPanel;
	wxBoxSizer * mLibsSizer;
	wxCheckedListCtrl * mList;
	wxListBox * mPlayLists;
//	wxBoxSizer * mSizer1;
    wxDECLARE_EVENT_TABLE();
	PlayerApp * mApp;
};


