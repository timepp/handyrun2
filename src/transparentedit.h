#pragma once

#include "helper.h"

class CTransparentEdit : public CWindowImpl<CTransparentEdit, CEdit>
{
public:
	BEGIN_MSG_MAP(CTransparentEdit)
		MESSAGE_RANGE_HANDLER(WM_KEYFIRST, WM_KEYLAST, OnKey)
	END_MSG_MAP()

	void SetSuggestWnd(HWND hwnd)
	{
		m_suggest_wnd = hwnd;
	}

private:
	CWindow m_suggest_wnd;

	LRESULT OnKey(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled)
	{
		handled = FALSE;

		// TODO: 当在非模态对话框中的编辑框里按ESC或回车时,会有响声。不知道
		// 手工处理一下是不是最正确的解决办法
		if (wp == VK_ESCAPE || wp == VK_RETURN)
		{
			handled = TRUE;
		}

		if (m_suggest_wnd.IsWindow() && m_suggest_wnd.IsWindowVisible())
		{
			m_suggest_wnd.SendMessageW(msg, wp, lp);
			if (wp == VK_UP || wp == VK_DOWN)
			{
				handled = TRUE;
			}
		}
		return 0;
	}
};