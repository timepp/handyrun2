#pragma once

#include "resource.h"

class CImportProgDlg : public CDialogImpl<CImportProgDlg>
{
public:
	enum {IDD = IDD_IMPORT_PROG};
	BEGIN_MSG_MAP(CImportProgDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDC_BUTTON_SELECT_PATH, BN_CLICKED, OnBnClickedButtonSelectPath)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSelectPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};