#include "stdafx.h"
#include "edgedlg.h"

CEdgeDlg::CEdgeDlg() 
: m_hwndRunDlg(NULL)
{
}

void CEdgeDlg::SetRunDlg(HWND hwndRunDlg)
{
	m_hwndRunDlg = hwndRunDlg;	
}

///////////////////////////////////////////////////////////
LRESULT CEdgeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	// ���ô�С��λ��

	return 0;
}