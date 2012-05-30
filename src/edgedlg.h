#pragma once

#include "resource.h"
#include "custombkdlg.h"

#include <string>


/** Edge Dialog
 *  ��ʾ����Ļ��Ե�ĸ�Ӧ��������Ƶ�egde dialog
 *  ��ʱ�����д��ڱ�����
 */

class CEdgeDlg : public CDialogImpl<CEdgeDlg>, public CCustomBkDlgImpl<CEdgeDlg>
{
public:
	enum {IDD = IDD_EDGEDLG};
	CEdgeDlg();

	// �������жԻ�����
	void SetRunDlg(HWND hwndRunDlg);

private:
	HWND m_hwndRunDlg;

	BEGIN_MSG_MAP(CEdgeDlg)
		CHAIN_MSG_MAP(CCustomBkDlgImpl<CEdgeDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
/*
		MESSAGE_HANDLER(WM_NCMOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
*/
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};