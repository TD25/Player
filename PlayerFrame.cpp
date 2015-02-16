#include "PlayerFrame.h"
#include "Common.h"
#include "File.h"

//constants
const int maxSliderVal = 1000;

// frame constructor
PlayerFrame::PlayerFrame(const wxString& title, wxPoint &pos, 
		wxSize &size, PlayerApp * pApp)
       : wxFrame(NULL, wxID_ANY, title, pos, size), mApp(pApp), 
       mActiveLib(0), mDontStoreSelection(false), mSliderTimer(this),
	   mCurrItemId(-1), mSecondsPlaying(0), 
	   mActivePlaylist(0), mCurrItemInList(-1)
{
    // set the frame icon
//    SetIcon(wxICON(sample));

#if wxUSE_MENUS
    // create a menu bar
    mFileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(ABOUT, "&About\tF1", "Show about dialog");

	mFileMenu->Append(SEARCH, "Search drive", "Search hard disk again");
	mFileMenu->Append(STOP_SEARCH, "Stop search", "Stop searching hard disk");
    mFileMenu->Append(QUIT, "E&xit\tAlt-X", "Quit this program");

   	// now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(mFileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUSB

    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);

	mCurrLengthStr = "0.0";

	wxBoxSizer * mainSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * sizer1 = new wxBoxSizer(wxHORIZONTAL);
	//create panel and assign it buttons
	
	SetBackgroundColour(wxTheColourDatabase->Find("DARK GREY"));

	mMediaCtrlsPanel = new wxPanel(this, wxID_ANY, 
			wxPoint(10, 10), wxSize(size.y, 50));
	mMediaCtrlsPanel->SetBackgroundColour(wxTheColourDatabase->
			Find("DARK GREY"));

	wxPoint btPos(10, 10);
	wxSize rBtSize(50, 50);
	wxSize btSize = rBtSize;

	wxButton * backBt = new wxButton(mMediaCtrlsPanel, CTRL_REVERSE, "<<",
			btPos, btSize);
	sizer1->AddSpacer(10);
	sizer1->Add(backBt, 0, wxALIGN_CENTER | wxSHAPED);
	btPos.x += btSize.x;
	mPlayBt = new wxButton(mMediaCtrlsPanel, CTRL_PLAY, ">", 
			btPos, btSize);
	sizer1->Add(mPlayBt, 0, wxALIGN_CENTER | wxSHAPED);
	btPos.x += btSize.x;
	wxButton * forwardBt = new wxButton(mMediaCtrlsPanel, CTRL_FORWARD, ">>",
			btPos, btSize);
	sizer1->Add(forwardBt, 0, wxALIGN_CENTER | wxSHAPED);
	btPos.x += btSize.x + 20;
	btPos.y += 10;
	sizer1->AddSpacer(10);
	mVolButton = new wxButton(mMediaCtrlsPanel, CTRL_VOL_BUTTON, "VOL",
			btPos, wxSize(25, 25));
	mVolPopup = new wxPopupTransientWindow(this);
	wxPanel * volPanel = new wxPanel(mVolPopup, wxID_ANY,
			wxPoint(0, 0), wxSize(25, 100));
	mVolSlider = new wxSlider(volPanel, CTRL_VOL_SLIDER, 0, 0, 
			SLIDER_MAX_VAL, wxPoint(5, 1), wxSize(20, 100), 
			wxVERTICAL | wxSL_INVERSE);
	mVolSlider->SetValue(SLIDER_MAX_VAL); 
	sizer1->Add(mVolButton, 0, wxALIGN_CENTER);
	sizer1->AddSpacer(10);	
	btPos.x += btSize.x + 20;
	wxPanel * sliderTextPan = new wxPanel(mMediaCtrlsPanel, wxID_ANY);
	sliderTextPan->SetBackgroundColour(wxTheColourDatabase->
			Find("DARK GREY"));
	wxBoxSizer * sliderTextSizer = new wxBoxSizer(wxVERTICAL);
	mSlider = new wxSlider(sliderTextPan, CTRL_SLIDER, 0, 0,
		   	maxSliderVal, wxPoint(btPos.x, btPos.y+5), wxSize(400, 20));
   	wxFont font;
	font.SetPointSize(14);
	mTextPanel = new TextPanel(sliderTextPan, wxPoint(0,0),
			wxSize(100,font.GetPixelSize().y+10), "", font, "WHITE", 
			wxPoint(0, 9));
	wxPanel * timeArtistPan = new wxPanel(sliderTextPan, wxID_ANY);
	timeArtistPan->SetBackgroundColour(wxTheColourDatabase->
			Find("DARK GREY"));
	wxBoxSizer * timeArtistSizer = new wxBoxSizer(wxHORIZONTAL);
	mTimePanel = new TextPanel(timeArtistPan, wxPoint(-1, -1),
			wxSize(100, 19));
	mTimePanel->mMode = TextPanel::RIGHT_SIDE;
	mArtistPanel = new TextPanel(timeArtistPan, wxPoint(-1, -1),
			wxSize(100, 19));
	mArtistPanel->SetBackgroundColour(wxTheColourDatabase->Find(
				"DARK GREY"));
	mTimePanel->SetBackgroundColour(wxTheColourDatabase->Find(
				"DARK GREY"));
	mTextPanel->SetBackgroundColour(wxTheColourDatabase->Find(
				"DARK GREY"));
	timeArtistSizer->Add(mArtistPanel, 1);
	timeArtistSizer->Add(mTimePanel, 1);
	timeArtistPan->SetSizerAndFit(timeArtistSizer);
	sliderTextSizer->Add(mTextPanel, 0, wxEXPAND);
	sliderTextSizer->Add(timeArtistPan, 0, wxEXPAND);
	sliderTextSizer->Add(mSlider, 0, wxEXPAND);
	sliderTextPan->SetSizerAndFit(sliderTextSizer);
	sizer1->Add(sliderTextPan, 6);
	wxPanel * picturePanel = new wxPanel(mMediaCtrlsPanel, wxID_ANY,
			wxPoint(-1, -1), wxSize(60, 60));
	sizer1->Add(picturePanel, 0);
	btPos.x = size.x - 200;
	btSize.y = 25;
	btSize.x = 200;
	mSearchBox = new wxTextCtrl(mMediaCtrlsPanel, CTRL_SEARCH, "",
			btPos, btSize, wxTE_PROCESS_ENTER);
	sizer1->Add(mSearchBox, 4, wxALIGN_CENTER);
	
	mMediaCtrlsPanel->SetSizerAndFit(sizer1);

//	mainSizer->Add(sizer1, 0, wxEXPAND);
//	mainSizer->AddSpacer(10);
	mainSizer->Add(mMediaCtrlsPanel, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL);
	
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
	mainSizer->AddSpacer(5);
	mainSizer->Add(mLibsPanel, 0, wxEXPAND); 
	//make playLists and media list panels
	//media list
	wxBoxSizer * sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxPanel * panel2 = new wxPanel(this, wxID_ANY, wxPoint(0, size.y-40),
			wxSize(size.x, size.y-40));
	panel2->SetBackgroundColour(wxTheColourDatabase->Find("DARK GREY"));

	//playlists
//	mPlayLists = new wxListBox(panel2, CTRL_PLAYLISTS, wxPoint(0, 0),
//			wxSize(size.x/5, size.y),0, NULL, wxLB_SINGLE | wxLB_NEEDED_SB |
//			wxLB_HSCROLL);
//	sizer2->Add(mPlayLists, 1, wxEXPAND);
//	wxString items[5] = {"item1", "item2", "item3", "item4", "item5"};
//	mPlayLists->InsertItems(5, items, 0);

	//media list
	mList = new ResCheckedListCtrl(panel2, CTRL_LIST, wxPoint(0, 0),
			wxSize(size.x, size.y - 40), wxLC_REPORT | wxLC_HRULES | 
			wxLC_VRULES);

	sizer2->Add(mList, 8, wxEXPAND);
	panel2->SetSizerAndFit(sizer2);
	mainSizer->AddSpacer(10);
	mainSizer->Add(panel2, 10, wxEXPAND);

	SetSizer(mainSizer);

	mOrgSize = size;
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

void PlayerFrame::AddLib(wxString label, wxString columns[], int n)
{
	static wxVector<wxButton*> buttons;

	mLibNames.push_back(label);
	mPlaylistNames.push_back(wxVector<wxString>());
	mPlaylistNames[mLibNames.size()-1].push_back("all");
	wxSize size(GetSize().x/(buttons.size()+1), 25);
	//resize others
	for (int i = 0; i < buttons.size(); i++)
	{
		wxSizerItem * item = mLibsSizer->GetItem(buttons[i]);
		item->SetMinSize(size);
	}	
	int num = buttons.size();
	buttons.push_back(new wxButton(mLibsPanel, 
				CTRL_LIB_SELECT_BUTTON+num, label, 
				wxPoint(), size));
	buttons.back()->SetBackgroundColour(wxTheColourDatabase->
			Find("DARK GREY"));
	buttons.back()->SetForegroundColour(wxTheColourDatabase->Find("WHITE"));
	mLibsSizer->Add(buttons.back(), 1, wxALIGN_CENTER | 
			wxALIGN_CENTER_HORIZONTAL);
	mLibsPanel->SetSizerAndFit(mLibsSizer);

	//register event
	Bind(wxEVT_BUTTON, &PlayerFrame::OnLibButton, this,
			CTRL_LIB_SELECT_BUTTON+num);
}


void PlayerFrame::AddListItem(const File * file)
{
	wxListItem item;
	item.SetId(mList->GetItemCount());
	item.SetText(file->GetName());
	mList->InsertItem(item);	
	if (mList->GetColumnCount() != file->GetColCount())
		throw MyException(
"PlayerFrame::AddListItem() col counts in list and file are different",
			MyException::FATAL_ERROR);
	if (file->GetColContent(0).size() == 0)
		mList->SetItem(item.GetId(), 0, file->GetName());
	for (int i = 1; i < mList->GetColumnCount(); i++)
	{
		mList->SetItem(item.GetId(), i, file->GetColContent(i));
	}
}

void PlayerFrame::OnNewItem(wxCommandEvent& evt)
{
	//TODO also check which playlist
	if (mSearchBox->GetValue().size() > 0)
		return;
	ListUpdateEv * ev = dynamic_cast<ListUpdateEv*>(&evt);
	assert(ev != NULL);
	const File * file = ev->GetFile();
	wxString lib = file->GetType();
	wxString plName = ev->GetPlaylistName();
	if (lib == mLibNames[mActiveLib] && 
			plName == mPlaylistNames[mActiveLib][mActivePlaylist])
	{
		mListMap.push_back(mList->GetItemCount());
		AddListItem(file);
	}
}

void PlayerFrame::OnVolButton(wxCommandEvent& ev) 
{
	wxPoint p = mVolButton->GetScreenPosition();
	p.y -= 50;
	mVolPopup->Position(p, wxSize(5, 5));
	mVolPopup->SetSize(wxSize(25, 100));
	mVolPopup->Popup();
}

void PlayerFrame::ShiftPlayBt(bool isPlaying)
{
	wxString curr = mPlayBt->GetLabel();
	if (isPlaying)
		mPlayBt->SetLabel("||");
	else
		mPlayBt->SetLabel(">");
}

long PlayerFrame::GetCurrSelection() const
{
	if (mSelectedItems.size() > 0)
	{
		int i = mSelectedItems[0];
		return mListMap[i];
	}
	else if (mCheckedItems.size() > 0)
		return mListMap[mCheckedItems[0]];
	else
		throw MyException("Nothing selected", MyException::NOT_FATAL);
}

void PlayerFrame::OnChecked(wxListEvent& ev)
{
	mCheckedItems.push_back(ev.GetIndex());
}

void PlayerFrame::OnUnchecked(wxListEvent& ev)
{
	long i = ev.GetIndex();
	int id = Find(mCheckedItems, i);
	if (id == -1)
//		throw MyException("No such index: PlayerFrame::OnUnchecked()",
//				MyException::NOT_FATAL);
	mCheckedItems.erase(mCheckedItems.begin() + id);
}

void PlayerFrame::OnSelected(wxListEvent& ev)
{
	if (mDontStoreSelection)
		return;
	mDontStoreSelection = false;
	int i = ev.GetIndex();
	mSelectedItems.push_back(i);
}

void PlayerFrame::OnDeselected(wxListEvent& ev)
{
	int id = Find(mSelectedItems, ev.GetIndex());
	if (id == -1)
		return;
//		throw MyException("No such index: PlayerFrame::OnDeselected()",
//				MyException::NOT_FATAL);

	mSelectedItems.erase(mSelectedItems.begin() + id);
}

void PlayerFrame::Select(const int & id)
{
	mDontStoreSelection = true;

	wxListEvent * ev = new wxListEvent(wxEVT_LIST_ITEM_SELECTED);
	ev->m_itemIndex = id;
	wxQueueEvent(this, ev);
}

void PlayerFrame::Deselect(const int & id)
{
	wxListEvent * ev = new wxListEvent(wxEVT_LIST_ITEM_DESELECTED);	
	ev->m_itemIndex = id;
	wxQueueEvent(this, ev);
}

void PlayerFrame::OnSliderTimer(wxTimerEvent& ev)
{
	mSlider->SetValue(mSlider->GetValue()+1);
}

void PlayerFrame::DeleteCurrSelection()
{
	if (mSelectedItems.size() > 0)
	{
		int i = Find(mCheckedItems, mSelectedItems[0]);
		if (i > -1)
		{
			mList->Check(mCheckedItems[i], false);
			mCheckedItems.erase(mCheckedItems.begin()+i);
		}
		mSelectedItems.erase(mSelectedItems.begin());
	}
	else if (mCheckedItems.size() > 0)
		mCheckedItems.erase(mCheckedItems.begin());		
}

void PlayerFrame::OnPaint(wxPaintEvent& ev)
{
	if (mCurrItemId > -1)
		DrawName();
	DrawTime();
}

void PlayerFrame::DrawName()
{
	if (mCurrItemId == -1)
		throw MyException("mCurrItemId is not set", 
				MyException::NOT_FATAL);
	const File * file = GetCurrFile();
	const MediaFile * aFile;
	//TODO: test for pictures as well
	if (file->GetType() == "Music" || file->GetType() == "Video")
		aFile = dynamic_cast<const MediaFile*>(file);
	wxString str;
	if (aFile != nullptr)
	{
		str = aFile->GetTitle();
		wxString artist = aFile->GetArtist();
		mArtistPanel->ChangeText(artist);
		
	}	
	else
	{
		str = file->GetName();
		mArtistPanel->ChangeText("");
	}
	mTextPanel->ChangeText(str);
}

void PlayerFrame::DrawTime()
{
	if (mCurrItemId == -1)
	{
		mTimePanel->ChangeText("0:0/0:0");
	}
}


long PlayerFrame::GetCurrSelectionInList() const
{
	if (mSelectedItems.size() > 0)
	{
		return mSelectedItems[0];
	}
	else if (mCheckedItems.size() > 0)
		return mCheckedItems[0];
	else
		throw MyException("Nothing selected", MyException::NOT_FATAL);

}

void PlayerFrame::MakeColumns(const wxString & lib)
{
	File * file;
	if (lib == "Music")
		file = new MusicFile;
	else if (lib == "Video")
		file = new VideoFile;
	else 
		return;
	int n = file->GetColCount();
	//get sizes sum for 
	//calculating real sizes from relative ones
	int sizeSum = 0;
	for (int i = 0; i < n; i++)
		sizeSum += file->GetColSize(i);
	for (int i = 0; i < n; i++)
	{
		int size = ((double)file->GetColSize(i)/sizeSum)*
			mList->GetSize().x;
		mList->AppendColumn(file->GetCol(i), wxLIST_FORMAT_LEFT, size);
	}
	delete file;
}

void PlayerFrame::OnLibButton(wxCommandEvent& ev)
{
	int id = ev.GetId();	
	mActiveLib = id - CTRL_LIB_SELECT_BUTTON;
	mActivePlaylist = 0;
	mList->DeleteAllItems();
	mList->DeleteAllColumns();
	MakeColumns(mLibNames[mActiveLib]);
	OnSearch(ev);
}
