#include "stdafx.h"
#include "icondlg.h"
#include "helper.h"
#include "defs.h"

CIconDlg::CIconDlg(const std::wstring &icon_file, int icon_index)
: m_icon_file(icon_file), m_icon_index(icon_index)
{
}

int CIconDlg::GetSelectedIndex() const
{
	return m_icon_index;
}

LRESULT CIconDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_icon_index = m_list.GetSelectedIndex();
	if (m_icon_index == -1)
	{
		m_icon_index = m_list.GetNextItem(-1, LVNI_FOCUSED);
	}
	if (m_icon_index == -1) m_icon_index = 0;

	EndDialog(IDOK);
	return 0;
}

LRESULT CIconDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CIconDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
{
	DlgResize_Init(true);

	m_list = GetDlgItem(IDC_LIST_ICON);
	m_list.ModifyStyleEx(0, LVS_EX_BORDERSELECT);
	m_list.SetIconSpacing(60, 60);

	CImageList il = m_list.GetImageList(LVSIL_NORMAL);
	if (il.IsNull())
	{
		il.Create(32, 32, ILC_COLOR32 | ILC_MASK, 4, 4);
		il.SetBkColor(CLR_NONE);
		m_list.SetImageList(il, LVSIL_NORMAL);
	}
	il.RemoveAll();

	UINT count = ExtractIconEx(m_icon_file.c_str(), -1, NULL, NULL, 0);
	if (count == 0)
	{
		HICON icon = hlp::get_file_icon(m_icon_file.c_str());
		if (icon != NULL)
		{
			il.AddIcon(icon);
			count = 1;
			DestroyIcon(icon);
		}
	}
	else
	{
		HICON * arr_icon = new HICON[count];
		ExtractIconEx(m_icon_file.c_str(), 0, arr_icon, NULL, count);
		for (UINT i = 0; i < count; i++)
		{
			il.AddIcon(arr_icon[i]);
			DestroyIcon(arr_icon[i]);
		}
		delete [] arr_icon;
	}

	wchar_t buf[16];
	for (UINT i = 0; i < count; i++)
	{
		_itow_s(i, buf, elem_of(buf), 10);
		m_list.InsertItem(i, buf, i);
	}
	m_list.SelectItem(m_icon_index);
	m_list.SetFocus();
	return FALSE;
}
