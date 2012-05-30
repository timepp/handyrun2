#pragma once
#include <string>
#include <windows.h>

// global data
namespace gd
{
	extern std::wstring hr_path;
	extern std::wstring hr_ini;
	extern HWND prev_active_wnd;
	extern bool run_once;
};

// string resources
namespace sr
{
	extern const wchar_t * default_grp_name;
};