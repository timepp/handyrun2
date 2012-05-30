#pragma once

#include "resource.h"
#include "prog.h"

class CProgDlg : public CDialogImpl<CProgDlg>
{
private:
	prog & m_pi;
	CStatic m_static_icon;
	CComboBox m_cb;
	CRect m_rc_full;
	CRect m_rc_fold;
	int m_icon_index;
	bool m_initialized;
private:
	void UpdateValue(UINT id, std::wstring * str);
	void ShowIcon();
	std::wstring get_peb_string(HANDLE proc, char * addr);
	void check_for_toggle_path(UINT id, POINT pt);
	void toggle_show(bool full);
public:
	enum {IDD = IDD_PROG};
	CProgDlg(prog & pi);

	BEGIN_MSG_MAP(CProgDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BUTTON_SELECT_PROG, BN_CLICKED, OnBnClickedButtonSelectProg)
		COMMAND_HANDLER(IDC_BUTTON_SELECT_WORKING_DIR, BN_CLICKED, OnBnClickedButtonSelectWorkingDir)
		COMMAND_HANDLER(IDC_BUTTON_SELECT_ICON_FILE, BN_CLICKED, OnBnClickedButtonSelectIconFile)
		COMMAND_HANDLER(IDC_BUTTON_GET_FGP, BN_CLICKED, OnBnClickedButtonGetFgp)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		COMMAND_HANDLER(IDC_EDIT_PROG_NAME, EN_CHANGE, OnEnChangeEditProgName)
		COMMAND_HANDLER(IDC_BUTTON_TOGGLE_FOLD, BN_CLICKED, OnBnClickedButtonToggleFold)
		COMMAND_HANDLER(IDC_STATIC_ICON, STN_CLICKED, OnStnClickedStaticIcon)
		COMMAND_HANDLER(IDC_COMBO_ICON_FILE, CBN_EDITCHANGE, OnCbnEditchangeComboIconFile)
		COMMAND_HANDLER(IDC_COMBO_ICON_FILE, CBN_EDITUPDATE, OnCbnEditupdateComboIconFile)
		COMMAND_HANDLER(IDC_COMBO_ICON_FILE, CBN_SELCHANGE, OnCbnSelchangeComboIconFile)

	END_MSG_MAP()
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedButtonSelectProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSelectWorkingDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSelectIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonGetFgp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditProgName(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonToggleFold(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStnClickedStaticIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnEditchangeComboIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnEditupdateComboIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelchangeComboIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
