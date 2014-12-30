#pragma once
#include "checkedlistctrl.h"
//list with automaticaly resizing columns
//it determines relative sizes on resize events and resizes columns 
	//to fill whole width
class ResCheckedListCtrl : public wxCheckedListCtrl
{
public:
	ResCheckedListCtrl()
        : wxCheckedListCtrl() {}

    ResCheckedListCtrl(wxWindow *parent, wxWindowID id = -1,
                        const wxPoint& pt = wxDefaultPosition,
                        const wxSize& sz = wxDefaultSize,
                        long style = wxCLC_CHECK_WHEN_SELECTING,
                        const wxValidator& validator = wxDefaultValidator,
                        const wxString& name = wxListCtrlNameStr)
                        : wxCheckedListCtrl(parent, id, pt, sz,
								style, validator, name)
	{}
	//call this with relative width
//	void AppendRelSizeColumn(const wxString & label, 
//			wxListColumnFormat format, int relWidth);
	void OnResize(wxSizeEvent & ev);
	wxDECLARE_EVENT_TABLE();
};

