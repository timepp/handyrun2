#include "stdafx.h"
#include "path_lister.h"
#include "error.h"

path_lister::path_lister(const wchar_t *path, int depth)
: m_base_path(path)
, m_max_depth(depth)
{
	if (can_do_deeper_search())
	{
		list_state ls = {L"", NULL};
		m_lss.push(ls);
	}
}

path_lister::~path_lister()
{
	while (m_lss.size() > 0)
	{
		const list_state& ls = m_lss.top();
		if (ls.hp) ::FindClose(ls.hp);
		m_lss.pop();
	}
}

bool path_lister::can_do_deeper_search() const
{
	return (m_max_depth == 0 || m_max_depth > static_cast<int>(m_lss.size()));
}

bool path_lister::get_next_file(std::wstring &subpath, std::wstring &filename)
{
	WIN32_FIND_DATA wfd;
	while (m_lss.size() > 0)
	{
		list_state& ls = m_lss.top();
		if (ls.hp == NULL)
		{
			std::wstring find_str = m_base_path + L"\\" + ls.sub_path + L"\\*";
			ls.hp = ::FindFirstFile(find_str.c_str(), &wfd);
			if (ls.hp == INVALID_HANDLE_VALUE) 
			{
				// 当前目录搜索失败, 返回
				// TODO: 记录搜索失败的信息供以后查询
				m_lss.pop();
				continue;
			}
		}
		else
		{
			BOOL ret = ::FindNextFile(ls.hp, &wfd);
			if (ret == 0)
			{
				::FindClose(ls.hp);
				m_lss.pop();
				continue;
			}
		}

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (can_do_deeper_search())
			{
				if (wcscmp(wfd.cFileName, L".") != 0 && wcscmp(wfd.cFileName, L"..") != 0)
				{
					list_state new_ls = {ls.sub_path + L"\\" + wfd.cFileName, NULL};
					m_lss.push(new_ls);
				}
			}
		}
		else
		{
			subpath = ls.sub_path;
			filename = wfd.cFileName;
			return true;
		}
	}
	return false;
}

