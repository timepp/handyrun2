#pragma once

#include "resource.h"
#include "prog.h"
#include "codesuggestpane.h"
#include <string>

/* 用来显示(命令)提示的对话框
   特征：永远没有自已的焦点
*/
class CTipDlg : public CDialogImpl<CTipDlg>, public CMessageFilter
{
public:
	enum {IDD = IDD_TIP};
	BEGIN_MSG_MAP(CTipDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_NCACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivate)
		MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
		NOTIFY_HANDLER(IDC_LIST, NM_CUSTOMDRAW, OnNMCustomdrawList)
		NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, OnNMDblclkList)
	END_MSG_MAP()

	void SetPair(HWND pair);
	void OnInputChange();
	command * GetSelectCmd() const;
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
	CCodeSuggestionPane m_list;
	HWND m_pair;
	CImageList m_il;
	//CListViewCtrl m_list;

	void MoveSel(int offset);
	void Resize();
	void ResizeList();
	void add_cmd(command * c, int image, int pos = -1);

	LRESULT OnMouseActivate(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnInitDialog(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnKeyDown(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnCreate(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnDestroy(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnActivate(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnSize(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnExitSizeMove(UINT msg, WPARAM wp, LPARAM lp, BOOL& handled);
	LRESULT OnNMCustomdrawList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
public:
	LRESULT OnNMDblclkList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};
