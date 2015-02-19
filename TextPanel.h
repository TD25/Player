#pragma once
#include "wx/panel.h"
#include "wx/dcclient.h"
#include "wx/settings.h"

class TextPanel : public wxPanel
{
private:
	wxString mText;
	wxFont mFont;
	wxString mColor;
	wxPoint mPos;
public:
	TextPanel(wxWindow* parent) : wxPanel(parent), mText(""), 
		mColor("WHITE"), mPos(0, 0), 
		mFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)) {}
	TextPanel(wxWindow* parent, const wxString & text, 
			const wxString & col = "WHITE",
			const wxPoint textPos = wxPoint(0, 0)) : wxPanel(parent),
		mFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)),
		mText(text), mColor(col), mPos(textPos) {}
	TextPanel(wxWindow * parent, const wxString & text, 
			const wxFont & font, 
			const wxString & col = "WHITE", 
			const wxPoint & textPos = wxPoint(0, 0)) : mText(text), 
			wxPanel(parent), mFont(font), mColor(col), mPos(textPos) {}
	TextPanel(wxWindow * parent, const wxPoint & pos, const wxSize & size,
			const wxString & text = "", const wxFont & font = 
			wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT),
			const wxString & col = "WHITE", 
			const wxPoint & textPos = wxPoint(0, 0)) : mText(text),
		wxPanel(parent, wxID_ANY, pos, size), mColor(col), mFont(font),
		mPos(textPos) {}
	void MySetFont(const wxFont & font)
	{
		mFont = font;
	}
	void SetColor(const wxString & col)
	{
		mColor = col;
	}
	void SetText(const wxString & text)
	{
		mText = text;
	}
	void SetPos(const wxPoint & pos)
	{
		mPos = pos;
	}
	void OnPaint(wxPaintEvent & ev);
	void ChangeText(const wxString & text) //updates immediately
	{
		mText = text;
		Refresh();
	}
	wxString GetText() const
	{
		return mText;
	}
	enum Aligment //text aligment
	{
		TOP_LEFT, TOP_RIGHT
	} mAligment = TOP_LEFT;
	//if mode is set aligment does not work
	//you can use mode to specify corner where to put text, instead of
	//	setting text pos
	enum Mode
	{
		NORMAL,	//text position is mPos
		RIGHT_SIDE //y coordinate remains the same
	} mMode = NORMAL;
	wxDECLARE_EVENT_TABLE();
};


