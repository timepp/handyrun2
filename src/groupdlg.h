#pragma once

#include "resource.h"
#include "prog.h"

class CGroupDlg : public CDialogImpl<CGroupDlg>
{
public:
	enum {IDD = IDD_GROUP};
	CGroupDlg(group_info & gi, bool add);

	bool m_insert_before;
private:
	group_info & m_gi;
	COLORREF m_color;
	bool m_add;

	BEGIN_MSG_MAP(CGroupDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_GROUP_COLOR, STN_CLICKED, OnStnClickedGroupColor)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnStnClickedGroupColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};