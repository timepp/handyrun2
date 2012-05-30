#include "stdafx.h"
#include "groupdlg.h"
#include "helper.h"
#include "defs.h"

CGroupDlg::CGroupDlg(group_info & gi, bool add) : m_gi(gi), m_add(add)
{
	m_color = gi.color;
	m_insert_before = false;
}

LRESULT CGroupDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_gi.color = m_color;
	wchar_t buf[256];
	UINT count = GetDlgItemText(IDC_EDIT_GROUP_NAME, buf, elem_of(buf));
	m_gi.name.assign(buf, count);
	m_insert_before = ::SendMessage(GetDlgItem(IDC_CHECK_INSERT_BEFORE), BM_GETCHECK, 0, 0) == BST_CHECKED;
	EndDialog(IDOK);
	return 0;
}

LRESULT CGroupDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CGroupDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
{
	SetDlgItemTextW(IDC_EDIT_GROUP_NAME, m_gi.name.c_str());
	GetDlgItem(IDC_CHECK_INSERT_BEFORE).ShowWindow(m_add? SW_SHOW : SW_HIDE);
	CenterWindow();
	return TRUE;
}

LRESULT CGroupDlg::OnStnClickedGroupColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(m_color, CC_FULLOPEN, m_hWnd);
	if (dlg.DoModal() == IDOK)
	{
		m_color = dlg.GetColor();
		RedrawWindow();
	}
	return 0;
}

LRESULT CGroupDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);
	CRect rc = hlp::get_dlg_item_rect(m_hWnd, IDC_GROUP_COLOR);
	rc.DeflateRect(2, 2);
	dc.FillSolidRect(rc, m_color);
	return 0;
}
