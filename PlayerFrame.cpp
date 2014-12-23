#include "PlayerFrame.h"
#include "Common.h"

//constants
const int maxSliderVal = 1000;

// frame constructor
PlayerFrame::PlayerFrame(const wxString& title, wxPoint &pos, 
		wxSize &size, PlayerApp * pApp)
       : wxFrame(NULL, wxID_ANY, title, pos, size), mApp(pApp), 
       mActiveLib(0), mDontStoreSelection(false), mSliderTimer(this)
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

    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);


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
	sizer1->Add(backBt, 1, wxALIGN_CENTER);
	btPos.x += btSize.x;
	mPlayBt = new wxButton(mMediaCtrlsPanel, CTRL_PLAY, ">", 
			btPos, btSize);
	sizer1->Add(mPlayBt, 1, wxALIGN_CENTER);
	btPos.x += btSize.x;
	wxButton * forwardBt = new wxButton(mMediaCtrlsPanel, CTRL_FORWARD, ">>",
			btPos, btSize);
	sizer1->Add(forwardBt, 1, wxALIGN_CENTER);
	btPos.x += btSize.x + 20;
	btPos.y += 10;
	sizer1->AddSpacer(10);
	mVolButton = new wxButton(mMediaCtrlsPanel, CTRL_VOL_BUTTON, "VOL",
			btPos, wxSize(25, 25));
	mVolPopup = new wxPopupTransientWindow(this);
	wxPanel * volPanel = new wxPanel(mVolPopup, wxID_ANY,
			wxPoint(0, 0), wxSize(25, 100));
	mVolSlider = new wxSlider(volPanel, CTRL_VOL_SLIDER, 0, 0, maxSliderVal,
		   wxPoint(5, 1), wxSize(20, 100), wxVERTICAL | wxSL_INVERSE);
	
	sizer1->Add(mVolButton, 0, wxALIGN_CENTER);
	sizer1->AddSpacer(10);	
	btPos.x += btSize.x + 20;
	mSlider = new wxSlider(mMediaCtrlsPanel, CTRL_SLIDER, 0, 0,
		   	maxSliderVal, wxPoint(btPos.x, btPos.y+5), wxSize(400, 20));
	mNamePos.x = btPos.x;
	mNamePos.y = 1;
	sizer1->Add(mSlider, 6, wxALIGN_BOTTOM);
	sizer1->AddSpacer(10);
	btPos.x = size.x - 200;
	btSize.y = 25;
	btSize.x = 200;
	wxTextCtrl * searchBox = new wxTextCtrl(mMediaCtrlsPanel, CTRL_SEARCH, "search",
			btPos, btSize);
	sizer1->Add(searchBox, 4, wxALIGN_CENTER);
	
	mMediaCtrlsPanel->SetSizerAndFit(sizer1);

//	mainSizer->Add(sizer1, 0, wxEXPAND);
	mainSizer->AddSpacer(10);
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
	mainSizer->AddSpacer(10);
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
	mList = new wxCheckedListCtrl(panel2, CTRL_LIST, wxPoint(0, 0),
			wxSize(size.x, size.y - 40), wxLC_REPORT | wxLC_HRULES | 
			wxLC_VRULES);

	mList->AppendColumn(wxT("Name"), wxLIST_FORMAT_LEFT, size.x / 1.5);     
	mList->AppendColumn(wxT("Size"), wxLIST_FORMAT_LEFT, size.x/8);

	sizer2->Add(mList, 8, wxEXPAND);
	panel2->SetSizerAndFit(sizer2);
	mainSizer->AddSpacer(10);
	mainSizer->Add(panel2, 10, wxEXPAND);

	SetSizer(mainSizer);
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
	wxSize size(GetSize().x/(buttons.size()+1), 25);
	//resize others
	for (int i = 0; i < buttons.size(); i++)
	{
		wxSizerItem * item = mLibsSizer->GetItem(buttons[i]);
		item->SetMinSize(size);
	}	
	buttons.push_back(new wxButton(mLibsPanel, wxID_ANY, label, 
				wxPoint(), size));
	buttons.back()->SetBackgroundColour(wxTheColourDatabase->
			Find("DARK GREY"));
	buttons.back()->SetForegroundColour(wxTheColourDatabase->Find("WHITE"));
	mLibsSizer->Add(buttons.back(), 1, wxALIGN_CENTER | 
			wxALIGN_CENTER_HORIZONTAL);
	mLibsPanel->SetSizerAndFit(mLibsSizer);
}


void PlayerFrame::AddListItem(wxListItem & item)
{
	mList->InsertItem(item);	
}

void PlayerFrame::OnNewItem(wxCommandEvent& evt)
{
	ListUpdateEv * ev = dynamic_cast<ListUpdateEv*>(&evt);
	assert(ev != NULL);
	wxListItem item;
	item.SetId(mList->GetItemCount());
	item.SetText(ev->GetFile().GetFullName());
	AddListItem(item);
}

void PlayerFrame::OnClose(wxCloseEvent & evt)
{
	Destroy();
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

int PlayerFrame::GetCurrSelection() const
{
	if (mSelectedItems.size() > 0)
	{
		int i = mSelectedItems[0];
		return i;
	}
	else if (mCheckedItems.size() > 0)
		return mCheckedItems[0];
	else
		throw MyException("Nothing selected", MyException::NOT_FATAL);
}

void PlayerFrame::OnChecked(wxListEvent& ev)
{
	mCheckedItems.push_back(ev.GetIndex());
}

void PlayerFrame::OnUnchecked(wxListEvent& ev)
{
	int i = ev.GetIndex();
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
	int i = ev.GetIndex();
	int id = Find(mSelectedItems, i);
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

void PlayerFrame::OnTimer(wxTimerEvent& ev)
{
	mSlider->SetValue(mSlider->GetValue()+1);
}
