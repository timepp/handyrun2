#pragma once


#define IDT_TIP_DELAY 1033

template <typename T>
class CRegionTipDlgImpl
{
public:
	BEGIN_MSG_MAP(CRegionTipDlgImpl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseAction)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, OnTTGetDispStub)
	END_MSG_MAP()

	CRegionTipDlgImpl(bool use_tip = true) : m_use_tip(use_tip)
	{
		m_show_tip = false;
	}

	void SetNewTip()
	{
		if (m_use_tip && m_tip_ctrl.IsWindow()) m_tip_ctrl.Pop();
	}

private:
	bool m_use_tip;
	CToolTipCtrl m_tip_ctrl;
	bool m_show_tip;

	virtual LRESULT OnTTGetDisp(CToolTipCtrl & tip_ctrl, LPNMHDR pnmh, BOOL &) = 0;

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = false;
		if (!m_use_tip) return 0;

		T * dlg = static_cast<T*>(this);
		m_tip_ctrl.Create(dlg->m_hWnd);
		m_tip_ctrl.SetMaxTipWidth(0);
		m_tip_ctrl.Activate(TRUE);
		// 哈哈，下面这行设置ReShow的时间为足够大
		// 成功解决了切到不同图标上有时tooltip会闪烁的问题
		m_tip_ctrl.SetDelayTime(TTDT_RESHOW, 10000);
		dlg->SetTimer(IDT_TIP_DELAY, 500);
	
		return 0;
	}

	LRESULT OnMouseAction(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = false;
		if (!m_use_tip || !m_tip_ctrl.IsWindow()) return 0;
		if (!m_show_tip) return 0;

		T * dlg = static_cast<T*>(this);
		MSG msg = {dlg->m_hWnd, uMsg, wParam, lParam};
		m_tip_ctrl.RelayEvent(&msg);
		return 0;
	}

	LRESULT OnShowWindow(UINT, WPARAM wp, LPARAM, BOOL& bHandled)
	{
		bHandled = FALSE;
		if (wp == TRUE)
		{
			T * dlg = static_cast<T*>(this);
			m_show_tip = false;
			dlg->SetTimer(IDT_TIP_DELAY, 500);
		}
		return 0;
	}

	LRESULT OnTimer(UINT , WPARAM wp, LPARAM , BOOL& bHandled)
	{
		bHandled = FALSE;
		if (wp == IDT_TIP_DELAY)
		{
			m_show_tip = true;
			T * dlg = static_cast<T*>(this);
			dlg->KillTimer(wp);
		}
		return 0;
	}

	LRESULT OnSize(UINT , WPARAM , LPARAM lParam, BOOL& bHandled)
	{
		bHandled = false;
		if (!m_use_tip || !m_tip_ctrl.IsWindow()) return 0;

		T * dlg = static_cast<T*>(this);
		RECT rc = {0, 0, LOWORD(lParam), HIWORD(lParam)};
		UINT flag;
		RECT old_rc;
		if (!m_tip_ctrl.GetToolInfo(dlg->m_hWnd, 1, &flag, &old_rc, NULL))
		{
			m_tip_ctrl.AddTool(dlg->m_hWnd, LPSTR_TEXTCALLBACK, &rc, 1);
		}
		else
		{
			m_tip_ctrl.SetToolRect(dlg->m_hWnd, 1, &rc);
		}

		return 0;
	}

	LRESULT OnTTGetDispStub(int , LPNMHDR pnmh, BOOL &bHandled)
	{
		return OnTTGetDisp(m_tip_ctrl, pnmh, bHandled);
	}
};
