#pragma once

#include "resource.h"
#include <WTL/atlctrlx.h>

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
	CHyperLink m_link;
public:
	enum {IDD = IDD_ABOUT};
	BEGIN_MSG_MAP(CAboutDlg)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		m_link.SubclassWindow(GetDlgItem(IDC_STATIC_URL));
		return TRUE;
	}
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDOK);
		return 1;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDCANCEL);
		return 1;
	}
};
