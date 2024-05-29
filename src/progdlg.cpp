#include "stdafx.h"
#include "progdlg.h"
#include "helper.h"
#include "defs.h"
#include "config.h"
#include "global_data.h"
#include "icondlg.h"

CProgDlg::CProgDlg(prog& pi) : m_pi(pi)
{
	m_initialized = false;
	m_icon_index = pi.icon.index;
}

void CProgDlg::UpdateValue(UINT id, std::wstring* str)
{
	wchar_t buf[1024];
	UINT count = GetDlgItemText(id, buf, elem_of(buf));
	str->assign(buf, count);
}

void CProgDlg::ShowIcon()
{
	if (!m_initialized) return;

	std::wstring icon_file;
	UpdateValue(IDC_COMBO_ICON_FILE, &icon_file);
	if (icon_file.empty()) UpdateValue(IDC_EDIT_PROG_NAME, &icon_file);
	icon_file = hlp::abs_path(icon_file.c_str());

	HICON icon = ExtractIcon(0, icon_file.c_str(), m_icon_index);
	if (icon == NULL)
	{
		icon = hlp::get_file_icon(icon_file.c_str());
	}
	::DestroyIcon(m_static_icon.GetIcon());
	m_static_icon.SetIcon(icon);
}

std::wstring CProgDlg::get_peb_string(HANDLE proc, char *addr)
{
	unsigned __int16 len;
	SIZE_T br;
	char *buffer;
	::ReadProcessMemory(proc, addr, &len, sizeof(len), &br);
	::ReadProcessMemory(proc, addr + 4, &buffer, sizeof(buffer), &br);
	wchar_t * path = new wchar_t[len /2 + 2];
	::ReadProcessMemory(proc, buffer, path, len, &br);
	path[len/2] = L'\0';
	std::wstring r = path;
	delete [] path;
	return r;
}

void CProgDlg::check_for_toggle_path(UINT id, POINT pt)
{
	RECT rc;
	GetDlgItem(id).GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (pt.y >= rc.top && pt.y <= rc.bottom)
	{
		std::wstring t;
		UpdateValue(id, &t);
		if (t.length() == 0) return;

		if (t.length() > 1 && t[1] == L':')
		{
			t = hlp::rela_path(t.c_str());
		}
		else
		{
			t = hlp::abs_path(t.c_str());
		}
		SetDlgItemText(id, t.c_str());
	}
}

LRESULT CProgDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (GetDlgItem(IDC_EDIT_PROG_NAME).GetWindowTextLengthW() == 0) return 0;

	UpdateValue(IDC_EDIT_PROG_NAME,   &m_pi.path);
	UpdateValue(IDC_EDIT_PARAM,       &m_pi.param);
	UpdateValue(IDC_EDIT_DROP_PARAM,  &m_pi.drop_param);
	UpdateValue(IDC_EDIT_WORKING_DIR, &m_pi.work_dir);
	UpdateValue(IDC_COMBO_ICON_FILE,  &m_pi.icon.file);
	UpdateValue(IDC_EDIT_COMMENT,     &m_pi.comment);
	m_pi.show_cmd = (int)m_cb.GetItemData(m_cb.GetCurSel());
	m_pi.icon.index = m_icon_index;

	// 如果图标文件和程序是一样的,则置为空
	if (hlp::abs_path(m_pi.path.c_str()) == hlp::abs_path(m_pi.icon.file.c_str()))
	{
		m_pi.icon.file = hc::empty_str;
	}
	// 如果工作目录就是程序所在目录,则置为空
	if (!m_pi.work_dir.empty())
	{
		std::wstring abspath = hlp::abs_path(m_pi.path.c_str());
		hlp::path_elem pe = hlp::split_path(abspath.c_str());
		std::wstring absdir = hlp::abs_path(m_pi.work_dir.c_str());
		if (*absdir.rbegin() != L'\\') absdir += L"\\";
		if (abspath.substr(0, pe.name.s) == absdir)
		{
			m_pi.work_dir = hc::empty_str;
		}
	}

	::DestroyIcon(m_static_icon.GetIcon());
	EndDialog(IDOK);
	return 0;
}

LRESULT CProgDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::DestroyIcon(m_static_icon.GetIcon());
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CProgDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
{
	//m_list.SetOutlineColor(RGB(255, 0, 0));
	m_cb = GetDlgItem(IDC_COMBO_RUN_TYPE);
	m_static_icon = GetDlgItem(IDC_STATIC_ICON);

	SetDlgItemText(IDC_EDIT_PROG_NAME, m_pi.path.c_str());
	SetDlgItemText(IDC_EDIT_PARAM, m_pi.param.c_str());
	SetDlgItemText(IDC_EDIT_DROP_PARAM, m_pi.drop_param.c_str());
	SetDlgItemText(IDC_EDIT_WORKING_DIR, m_pi.work_dir.c_str());
	SetDlgItemText(IDC_EDIT_COMMENT, m_pi.comment.c_str());
	SetDlgItemText(IDC_COMBO_ICON_FILE, m_pi.icon.file.c_str());

	const struct 
	{
		int show_cmd;
		const wchar_t * desc;
	} show_desc[] =
	{
		SW_SHOW,              L"正常显示",
		SW_SHOWMINNOACTIVE,   L"最小化",
		SW_SHOWMAXIMIZED,     L"最大化",
		SW_SHOWNOACTIVATE,    L"后台窗口",
		SW_HIDE,              L"隐藏",
	};
	int select = 0;
	for (int i = 0; i < elem_of(show_desc); i++)
	{
		m_cb.AddString(show_desc[i].desc);
		m_cb.SetItemData(i, show_desc[i].show_cmd);
		if (show_desc[i].show_cmd == m_pi.show_cmd) select = i;
	}
	m_cb.SetCurSel(select);

	CComboBox m_cbIcon = GetDlgItem(IDC_COMBO_ICON_FILE).m_hWnd;
	m_cbIcon.AddString(L"%windir%\\System32\\shell32.dll");

	GetClientRect(&m_rc_full);
	m_rc_fold = m_rc_full;
	m_rc_fold.bottom = hlp::get_dlg_item_rect(m_hWnd, IDC_STATIC_MIDLINE).bottom;

	GetDlgItem(IDC_EDIT_PROG_NAME).SetFocus();
	m_initialized = true;
	ShowIcon();
	return FALSE;
}

LRESULT CProgDlg::OnBnClickedButtonSelectProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::wstring path;
	UpdateValue(IDC_EDIT_PROG_NAME, &path);
	CFileDialog dlg(TRUE, NULL, hlp::abs_path(path.c_str()).c_str());
	if (dlg.DoModal() == IDOK)
	{
		path = hlp::rela_path(dlg.m_szFileName);
		SetDlgItemText(IDC_EDIT_PROG_NAME, path.c_str());
	}
	return 0;
}


LRESULT CProgDlg::OnBnClickedButtonSelectWorkingDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::wstring path;
	UpdateValue(IDC_EDIT_WORKING_DIR, &path);
	if (path.empty()) UpdateValue(IDC_EDIT_PROG_NAME, &path);
	path = hlp::abs_path(path.c_str());
	hlp::path_elem pe = hlp::split_path(path.c_str());
	path.resize(pe.name.s);

	CFolderDialog dlg(m_hWnd);
	dlg.SetInitialFolder(path.c_str());
	if (dlg.DoModal() == IDOK)
	{
		path = hlp::rela_path(dlg.m_szFolderPath);
		SetDlgItemText(IDC_EDIT_WORKING_DIR, path.c_str());
	}
	return 0;
}

LRESULT CProgDlg::OnBnClickedButtonSelectIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::wstring path;
	UpdateValue(IDC_COMBO_ICON_FILE, &path);
	if (path.empty()) UpdateValue(IDC_EDIT_PROG_NAME, &path);
	CFileDialog dlg(TRUE, NULL, hlp::abs_path(path.c_str()).c_str());
	if (dlg.DoModal() == IDOK)
	{
		path = hlp::rela_path(dlg.m_szFileName);
		SetDlgItemText(IDC_COMBO_ICON_FILE, path.c_str());
		ShowIcon();
	}

	return 0;
}

LRESULT CProgDlg::OnBnClickedButtonGetFgp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DWORD pid;
	GetWindowThreadProcessId(gd::prev_active_wnd, &pid);

	HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (proc == NULL) return 0;

	WCHAR path[MAX_PATH];
	DWORD pathlen = _countof(path);
	QueryFullProcessImageNameW(proc, 0, path, &pathlen);
	CloseHandle(proc);
	SetDlgItemText(IDC_EDIT_PROG_NAME, hlp::rela_path(path).c_str());
	
	return 0;
}

LRESULT CProgDlg::OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lp, BOOL& /*bHandled*/)
{
	POINT pt = {LOWORD(lp), HIWORD(lp)};
	check_for_toggle_path(IDC_EDIT_PROG_NAME, pt);
	check_for_toggle_path(IDC_EDIT_WORKING_DIR, pt);
	check_for_toggle_path(IDC_COMBO_ICON_FILE, pt);
	return 0;
}

LRESULT CProgDlg::OnEnChangeEditProgName(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::wstring path;
	UpdateValue(IDC_EDIT_PROG_NAME, &path);
	if (GetDlgItem(IDC_COMBO_ICON_FILE).GetWindowTextLengthW() == 0)
	{
		ShowIcon();
	}
	return 0;
}

LRESULT CProgDlg::OnStnClickedStaticIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::wstring icon_file;
	UpdateValue(IDC_COMBO_ICON_FILE, &icon_file);
	if (icon_file.empty()) UpdateValue(IDC_EDIT_PROG_NAME, &icon_file);
	icon_file = hlp::abs_path(icon_file.c_str());

	CIconDlg dlg(icon_file, m_icon_index);
	if (dlg.DoModal() == IDOK)
	{
		m_icon_index = dlg.GetSelectedIndex();
		ShowIcon();
	}
	return 0;
}


LRESULT CProgDlg::OnCbnEditchangeComboIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowIcon();
	return 0;
}


LRESULT CProgDlg::OnCbnEditupdateComboIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	return 0;
}


LRESULT CProgDlg::OnCbnSelchangeComboIconFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_COMMAND, MAKEWORD(IDC_COMBO_ICON_FILE, CBN_EDITCHANGE));
	return 0;
}
