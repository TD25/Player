#include "TextPanel.h"

void TextPanel::OnPaint(wxPaintEvent & ev)
{
	if (mText.size() == 0)
		return;
	wxPaintDC dc(this);
	dc.SetFont(mFont);
	dc.SetTextForeground(wxTheColourDatabase->Find(mColor));
	wxPoint pos;
	if (mMode == NORMAL)
	{
		if (mAligment == TOP_LEFT)
			pos = mPos;
		else if (mAligment == TOP_RIGHT)
		{
			int length = mText.size() * mFont.GetPixelSize().x;
			pos.x -= length;
			if (pos.x < 0)
				pos.x = 0;
		}
	}
	else if (mMode == RIGHT_SIDE)
	{
		pos = mPos;
		int x, y;
		dc.GetTextExtent(mText, &x, &y, NULL, NULL, &mFont);
		pos.x = dc.GetSize().x - x - 5;
	}  
	dc.DrawText(mText, pos);
}

wxBEGIN_EVENT_TABLE(TextPanel, wxPanel)
	EVT_PAINT(TextPanel::OnPaint)
wxEND_EVENT_TABLE()


