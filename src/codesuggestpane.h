#pragma once

#include "helper.h"

class CCodeSuggestionPane : 
	public CWindowImpl<CCodeSuggestionPane, CListViewCtrl> 
{
public:
	BEGIN_MSG_MAP(CCodeSuggestionPane)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseAction)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseAction)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseAction)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnMouseAction)
//		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouseAction)
	END_MSG_MAP()

private:
	LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&)
	{
		return MA_NOACTIVATE;
	}
	LRESULT OnMouseAction(UINT msg, WPARAM , LPARAM lp, BOOL& handled)
	{
		if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN)
		{
			int xPos = GET_X_LPARAM(lp);
			int yPos = GET_Y_LPARAM(lp);
			POINT pt = {xPos, yPos};
			int item = this->HitTest(pt, NULL);
			if (item >= 0)
			{
				this->SelectItem(item);
			}
		}

		handled = TRUE;
		return 0;
	}
};

