#pragma once

#include "resource.h"

class CIconDlg : public CDialogImpl<CIconDlg>, public CDialogResize<CIconDlg>
{
public:
	enum {IDD = IDD_ICON};
	CIconDlg(const std::wstring& icon_file, int icon_index);
	int GetSelectedIndex() const;

	BEGIN_MSG_MAP(CProgDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		CHAIN_MSG_MAP(CDialogResize<CIconDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CProgDlg)
		DLGRESIZE_CONTROL(IDC_LIST_ICON, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	std::wstring m_icon_file;
	int m_icon_index;
	CListViewCtrl m_list;
	void SetListIcon(int select_index);
};
