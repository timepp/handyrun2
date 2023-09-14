#include "stdafx.h"
#include "importdlg.h"
#include "defs.h"
#include "path_lister.h"
#include "config.h"
#include "error.h"
#include "helper.h"

LRESULT CImportProgDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	try
	{
		wchar_t buf[MAX_PATH];
		cfg::config * c = cfg::config::instance();
		GetDlgItemText(IDC_EDIT_IMPORT_PATH, buf, elem_of(buf));
		std::wstring abs_path = hlp::abs_path(buf, false);
		while (!abs_path.empty() && *abs_path.rbegin() == L'\\')
		{
			abs_path.resize(abs_path.size() - 1);
		}
		int depth = GetDlgItemInt(IDC_EDIT_DEPTH);
		
		FILE * fp;
		std::wstring fn = hlp::get_tmp_file_name(L".txt");
		if (_wfopen_s(&fp, fn.c_str(), L"w, ccs=UTF-8") != 0)
		{
			os_err oe(dos, L"写文件", L"  文件名:%s", fn);
			hlp::show_err(oe.what(), NULL);
			return 0;
		}

		fwprintf(fp, L"可以直接将以下行粘贴到配置文件中以实现导入功能\n------------\n");

		path_lister pl(abs_path.c_str(), depth);
		std::wstring subpath, filename;
		while (pl.get_next_file(subpath, filename))
		{
			hlp::path_elem pe = hlp::split_path(filename.c_str());
			if (pe.ext.n > 0 &&
				hlp::match_str_list(c->go.executable_ext.c_str(), filename.c_str() + pe.ext.s + 1) >= 0)
			{
				std::wstring path = abs_path + subpath + L"\\" + filename;
				path = hlp::rela_path(path.c_str());
				fwprintf(fp, L"%s\n", path.c_str());
			}
		}

		fclose(fp);
		ShellExecute(m_hWnd, L"open", fn.c_str(), NULL, NULL, SW_SHOW);
		EndDialog(IDOK);
	} 
	catch (handyrun_err& e)
	{
		hlp::show_err(e.what(), NULL);
	}
	return 0;
}

LRESULT CImportProgDlg::OnBnClickedButtonSelectPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	wchar_t buf[MAX_PATH];
	GetDlgItemText(IDC_EDIT_IMPORT_PATH, buf, elem_of(buf));
	std::wstring abs_path = hlp::abs_path(buf, false);

	CFolderDialog dlg(m_hWnd);
	dlg.SetInitialFolder(abs_path.c_str());
	if (dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_EDIT_IMPORT_PATH, dlg.m_szFolderPath);
	}
	return 0;
}

LRESULT CImportProgDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CImportProgDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
{
	SetDlgItemText(IDC_EDIT_DEPTH, L"1");
	CenterWindow(GetParent());
	return TRUE;
}
