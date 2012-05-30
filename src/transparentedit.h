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

		// TODO: ���ڷ�ģ̬�Ի����еı༭���ﰴESC��س�ʱ,������������֪��
		// �ֹ�����һ���ǲ�������ȷ�Ľ���취
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