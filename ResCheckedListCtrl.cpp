#include "ResCheckedListCtrl.h"

wxBEGIN_EVENT_TABLE(ResCheckedListCtrl, wxCheckedListCtrl)
	EVT_SIZE(ResCheckedListCtrl::OnResize)
wxEND_EVENT_TABLE()


//void ResCheckedListCtrl::AppendRelSizeColumn(const wxString & label,
//		wxListColumnFormat format, int relWidth)
//{
//}

void ResCheckedListCtrl::OnResize(wxSizeEvent & ev)
{
	ev.Skip();
	int n = GetColumnCount();
	int sizeSum = 0;
	for (int i = 0; i < n; i++)
	{
		sizeSum += GetColumnWidth(i);
	}
	for (int i = 0; i < n; i++)
	{
		int size = ((double)GetColumnWidth(i) / sizeSum) * GetSize().x;
		SetColumnWidth(i, size);
	}
}
