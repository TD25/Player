#pragma once
#include "wx/panel.h"
#include "wx/dcclient.h"

class TextPanel : public wxPanel
{
private:
	wxString mText;
	wxFont mFont;
	wxString mColor;
	wxPoint mPos;
public:
	TextPanel(wxWindow* parent) : wxPanel(parent), mText(""), 
		mColor("WHITE"), mPos(0, 0)	{}
	TextPanel(wxWindow* parent, const wxString & text, 
			const wxString & col = "WHITE",
			const wxPoint textPos = wxPoint(0, 0)) : wxPanel(parent),
		mText(text), mColor(col), mPos(textPos) {}
	TextPanel(wxWindow * parent, const wxString & text, 
			const wxFont & font, 
			const wxString & col = "WHITE", 
			const wxPoint & textPos = wxPoint(0, 0)) : mText(text), 
			wxPanel(parent), mFont(font), mColor(col), mPos(textPos) {}
	TextPanel(wxWindow * parent, const wxPoint & pos, const wxSize & size,
			const wxString & text = "", const wxFont & font = wxFont(),
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
	void OnPaint(wxPaintEvent & ev)
	{
		wxPaintDC dc(this);
		dc.SetFont(mFont);
		dc.SetTextForeground(wxTheColourDatabase->Find(mColor));
		dc.DrawText(mText, mPos);
	}
	void ChangeText(const wxString & text) //updates immediately
	{
		mText = text;
		Refresh();
	}
	wxDECLARE_EVENT_TABLE();
};


