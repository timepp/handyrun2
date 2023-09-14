#pragma once

#include <windows.h>
#include <string>
#include <stack>
// 遍历目录及其子目录

class path_lister
{
public:
	path_lister(const wchar_t * path, int depth);
	~path_lister();
	bool get_next_file(std::wstring& subpath, std::wstring& filename);

private:
	struct list_state
	{
		std::wstring sub_path;
		HANDLE hp;
	};
	std::stack<list_state> m_lss;
	int m_max_depth;
	std::wstring m_base_path;

	// 能否进行更深搜索
	bool can_do_deeper_search() const;
};
