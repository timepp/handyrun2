#include "stdafx.h"
#include "inputdlg.h"
#include "defs.h"

LRESULT CInputDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
{
	if (!m_caption.empty())
	{
		SetWindowTextW(m_caption.c_str());
	}
	SetDlgItemTextW(IDC_INPUT_TIP, m_input_tip.c_str());
	SetDlgItemTextW(IDC_EDIT, m_value.c_str());
	SendDlgItemMessageW(IDC_EDIT, EM_SETSEL, 0, -1);
	::SetFocus(GetDlgItem(IDC_EDIT));
	CenterWindow();
	return 0;
}

LRESULT CInputDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	wchar_t buf[1024];
	GetDlgItemTextW(IDC_EDIT, buf, elem_of(buf));
	m_value = buf;
	EndDialog(IDOK);
	return 0;
}
LRESULT CInputDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDCANCEL);
	return 0;
}
