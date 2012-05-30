#include "stdafx.h"

#include "tipdlg.h"
#include "config.h"
#include "defs.h"

extern CAppModule _Module;

void CTipDlg::SetPair(HWND pair)
{
	m_pair = pair;
}

// 根据列表里的内容调整大小
void CTipDlg::Resize()
{
	RECT rc;
	m_list.GetItemRect(0, &rc, LVIR_BOUNDS);
	int height = m_list.GetItemCount() * (rc.bottom - rc.top + 1);
	if (height > 150) height = 150;

	m_list.GetClientRect(&rc);
	m_list.ResizeClient(rc.right - rc.left, height);
	m_list.GetWindowRect(&rc);
	height = rc.bottom - rc.top;

	GetClientRect(&rc);
	ResizeClient(rc.right - rc.left, height);
	ResizeList();
	::SetFocus(m_pair);
}

void CTipDlg::ResizeList()
{
	if (m_list.IsWindow())
	{
		RECT rc;
		GetClientRect(&rc);
		m_list.MoveWindow(&rc);
		m_list.GetClientRect(&rc);
		m_list.SetColumnWidth(0, rc.right - rc.left);
	}
}

LRESULT CTipDlg::OnSize(UINT , WPARAM , LPARAM , BOOL &)
{
	ResizeList();
	return TRUE;
}

LRESULT CTipDlg::OnExitSizeMove(UINT , WPARAM , LPARAM , BOOL &handled)
{
	handled = TRUE;
	::SetFocus(m_pair);
	return TRUE;
}

LRESULT CTipDlg::OnMouseActivate(UINT , WPARAM , LPARAM , BOOL &)
{
	return MA_NOACTIVATE;
}

LRESULT CTipDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL &)
{
	_Module.GetMessageLoop()->AddMessageFilter(this);
	//m_list = GetDlgItem(IDC_LIST);
	m_list.SubclassWindow(GetDlgItem(IDC_LIST));
	m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP|LVS_EX_BORDERSELECT);
	m_list.AddColumn(L"", 0);
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	m_il.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
	m_il.SetBkColor(CLR_NONE);
	HBITMAP bmp = AtlLoadBitmap(IDB_UD);
	m_il.Add(bmp); DeleteObject(bmp);

	cfg::config * c = cfg::config::instance();
	for (int i = 0; i < c->index_mgr.cs_count(); i++)
	{
		HICON icon = NULL;
		index_info *ii = c->get_index_info(i);
		if (!ii->icon.file.empty())
		{
			std::wstring path = hlp::abs_path(ii->icon.file.c_str());
			ExtractIconEx(path.c_str(), ii->icon.index, NULL, &icon, 1);
			if (icon == NULL)
			{
				icon = (HICON)LoadImage(NULL, path.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
			}
		}
		if (icon == NULL)
		{
			bmp = AtlLoadBitmap(IDB_QUICK_LAUNCH);
			m_il.Add(bmp); 
			DeleteObject(bmp);
		}
		else
		{
			m_il.AddIcon(icon);
			DestroyIcon(icon);
		}
	}

	m_list.SetImageList(m_il, LVSIL_SMALL);

	return TRUE;
}

LRESULT CTipDlg::OnActivate(UINT , WPARAM , LPARAM , BOOL& handled)
{
	handled = TRUE;
	return TRUE;
}

LRESULT CTipDlg::OnCreate(UINT , WPARAM , LPARAM , BOOL &)
{
	return TRUE;
}

LRESULT CTipDlg::OnDestroy(UINT , WPARAM , LPARAM , BOOL &)
{
	_Module.GetMessageLoop()->RemoveMessageFilter(this);
	return 0;
}

BOOL CTipDlg::PreTranslateMessage(MSG *pMsg)
{
	if (pMsg->hwnd == m_pair)
	{
		if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
		{
			if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			{
				GetParent().PostMessageW(pMsg->message, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
			switch (pMsg->wParam)
			{
			case VK_UP:   MoveSel(-1); return TRUE;
			case VK_DOWN: MoveSel(1);  return TRUE;
			}
		}
		if (pMsg->message == WM_MOUSEWHEEL)
		{
			m_list.PostMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
		}
	}

	return FALSE;
}

void CTipDlg::add_cmd(command *c, int image, int pos)
{
	if (pos == -1) pos = m_list.GetItemCount();
	int index = m_list.InsertItem(pos, c->path.c_str(), image);
	m_list.SetItemData(index, (DWORD_PTR)(c));
}

void CTipDlg::OnInputChange()
{
	wchar_t input[256];
	size_t input_len = ::GetWindowTextW(m_pair, input, elem_of(input));

	m_list.DeleteAllItems();
	cfg::config * c = cfg::config::instance();
	int ind1 = 0;

	// 1. 图标应用程序
	for (int i = 0; i < c->group_mgr.cs_count(); i++)
	{
		for (int j = 0; j < c->group_mgr.cmd_count(i); j++)
		{
			command * cmd = c->get_prog(i, j);
			if (wcsnicmp(cmd->path.c_str(), input, input_len) == 0)
			{
				add_cmd(cmd, 0, ind1++);
			}
			else if (hlp::wcsistr(cmd->path.c_str(), input))
			{
				add_cmd(cmd, 0);
			}
		}
	}

	// 2. 只索引的应用程序
	for (int i = 0; i < c->index_mgr.cs_count(); i++)
	{
		for (int j = 0; j < c->index_mgr.cmd_count(i); j++)
		{
			command *cmd = c->get_index_cmd(i, j);
			if (wcsnicmp(cmd->path.c_str(), input, input_len) == 0)
			{
				add_cmd(cmd, i+1, ind1++);
			}
			else if (hlp::wcsistr(cmd->path.c_str(), input))
			{
				add_cmd(cmd, i+1);
			}
		}
	}

	if (m_list.GetItemCount() > 0)
	{
		if (!IsWindowVisible())
		{
			CRect rc;
			::GetWindowRect(m_pair, &rc);
			SetWindowPos(0, rc.left, rc.bottom + 5, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_SHOWWINDOW);
		}
		Resize();
		m_list.SelectItem(0);
	}
	else
	{
		ShowWindow(SW_HIDE);
	}
}

void CTipDlg::MoveSel(int offset)
{
	int c = m_list.GetItemCount();
	if (c > 0)
	{
		int i = m_list.GetSelectedIndex();
		i = (i + offset) % c;
		if (i < 0) i += c;
		m_list.SelectItem(i);
	}
}

command * CTipDlg::GetSelectCmd() const
{
	int i = m_list.GetSelectedIndex();
	if (i >= 0 && IsWindowVisible())
	{
		return (command *)m_list.GetItemData(i);
	}
	return 0;
}

LRESULT CTipDlg::OnNMCustomdrawList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		return CDRF_NOTIFYITEMDRAW;
	}
	if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT &&
		pNMCD->nmcd.dwItemSpec == (DWORD_PTR)m_list.GetSelectedIndex())
	{
		pNMCD->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT);
		pNMCD->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
	}
	return 0;
}

LRESULT CTipDlg::OnNMDblclkList(int /*idCtrl*/, LPNMHDR , BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	GetParent().SendMessageW(WM_COMMAND, IDOK);
	return 0;
}
