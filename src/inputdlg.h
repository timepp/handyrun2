#pragma once

#include "resource.h"
#include <string>

class CInputDlg : public CDialogImpl<CInputDlg>
{
public:
	enum {IDD = IDD_INPUT};
	CInputDlg(const wchar_t * caption, const wchar_t * tip, const wchar_t * initial_value)
		:m_caption(caption), m_input_tip(tip), m_value(initial_value)
	{
	}

	std::wstring get_value() const
	{
		return m_value;
	}
private:
	std::wstring m_caption;
	std::wstring m_input_tip;
	std::wstring m_value;
private:
	BEGIN_MSG_MAP(CImportProgDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT , WPARAM , LPARAM , BOOL& );
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};