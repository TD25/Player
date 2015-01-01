#pragma once

#include "wx/wx.h"
#include "ResCheckedListCtrl.h"
#include "wx/popupwin.h"
#include "wx/mediactrl.h"
#include "File.h"
#include "wx/timer.h"
#include "TextPanel.h"

#define SLIDER_MAX_VAL 1000
// IDs for the controls and the menu commands
enum
{
    // menu items
    QUIT = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    ABOUT = wxID_ABOUT,
	SEARCH,
	STOP_SEARCH,
	
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
	MEDIA_CTRL,
	TIMER_SLIDER,
	TIMER_TIME
};

class ListUpdateEv : public wxCommandEvent
{
private:
	const File * mpFile;
public:
	ListUpdateEv(const File * file);
	const File * GetFile() {return mpFile;}
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
	void AddListItem(const File * file);
	void OnNewItem(wxCommandEvent& event);
	void OnSearchCompletion(wxCommandEvent& event) 
	{
		wxLogStatus(this, "Search complete");
	}
	void OnClose(wxCloseEvent &);
	void OnVolButton(wxCommandEvent& ev);
	void OnMediaLoaded(wxMediaEvent& ev);
	void OnMediaStop(wxMediaEvent& ev) {}
	void OnMediaFinish(wxMediaEvent& ev);
	void OnPlayBt(wxCommandEvent& ev);
	void OnSearch(wxCommandEvent& ev);
	void OnForward(wxCommandEvent& ev);
	void OnReverse(wxCommandEvent& ev);
	void OnPlay(bool lookForNew = true);
	void OnListActivated(wxListEvent& ev);
	void OnSelected(wxListEvent& ev);
	void OnDeselected(wxListEvent& ev);
	void OnChecked(wxListEvent& ev);
	void OnUnchecked(wxListEvent& ev);
	void ShiftPlayBt(bool isPlaying);	//changes text (bitmap) on play button
	void Select(const int & id);
	void Deselect(const int & id);
	void OnSliderTimer(wxTimerEvent& ev);
	void OnSecondTimer(wxTimerEvent& ev);
	void DeleteCurrSelection();
	void OnPaint(wxPaintEvent& ev);
	void OnVolSlider(wxCommandEvent& ev);
	void OnSlider(wxScrollEvent& ev);
	void OnSearchDrive(wxCommandEvent& ev);
	void OnStopSearch(wxCommandEvent& ev);
	void DrawName();
	void DrawTime();
	void ResetSlider()
	{
		mSlider->SetValue(0);
	}
	void SetCurrLength(const wxString & length)
	{
		mCurrLengthStr = length;
	}
	void MakeColumns(const wxString & lib);
private:
	long GetCurrSelection() const;	//determines selection in the list
	long GetCurrSelectionInList() const;
	const File * GetCurrFile() const;

	long mCurrItemInList;
	wxMenu * mFileMenu;
	wxPanel * mLibsPanel;
	wxPanel * mMediaCtrlsPanel;
	TextPanel * mTextPanel;
	TextPanel * mArtistPanel;
	wxBoxSizer * mLibsSizer;
	ResCheckedListCtrl * mList;
	wxListBox * mPlayLists;
	wxSlider * mVolSlider;
	wxPopupTransientWindow * mVolPopup;
	wxSlider * mSlider;
	wxButton * mVolButton;
	wxButton * mPlayBt;
	wxTextCtrl * mSearchBox;
	wxVector<wxString> mLibNames;
	wxVector<wxVector<wxString>> mPlaylistNames;
	int mActivePlaylist;
	wxVector<long> mSelectedItems;
	wxVector<long> mCheckedItems;
	wxVector<long> mListMap;
	int mActiveLib;
	bool mDontStoreSelection;
//	wxBoxSizer * mSizer1;
    wxDECLARE_EVENT_TABLE();
	PlayerApp * mApp;
	wxTimer mSliderTimer;
	TextPanel * mTimePanel;
	long mCurrItemId; //currently played item's id in list
	int mSecondsPlaying;
	wxString mCurrLengthStr; //minutes and seconds
	friend PlayerApp;
	wxSize mOrgSize;
};


