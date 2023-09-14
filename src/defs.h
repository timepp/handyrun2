#pragma once

#include <vector>
#include <string>
#include <list>
#include <windows.h>

#ifndef elem_of
#define elem_of(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

typedef std::vector<void *> vvptr;
typedef std::list<std::wstring> string_list_t;
typedef std::list<COLORREF> color_list_t;

// hotkey结构描述一个热键(包括普通热键和双击热键)
struct hotkey
{
	UINT fk;        // 功能键(control,alt,shift,win的组合)
	                // 如果fk为0,描述一个"双击<vk>"的热键
	UINT vk;        // 键盘按键代码
};

// 额外的环境变量(由配置文件提供)
struct env_var
{
	std::wstring key;
	std::wstring val;
};

struct background
{
	enum bk_mode {bm_color, bm_img, bm_none};
	enum special_direction
	{
		sd_diag  = 1001,
		sd_rdiag = 1002
	};

	bk_mode mode;                     // 背景模式(图片或者填充)
	color_list_t color_list;          // (用于填充的)颜色列表
	int fill_direction;               // 填充角度，从0到360
	std::wstring back_img;
	std::wstring mask_img;

	background():mode(bm_color), fill_direction(0){}
};

enum value_type
{
	vt_int, vt_double, vt_wstring, vt_bool, vt_icon, vt_color, vt_direction, vt_color_list, vt_none
};

const double PI = 3.14159265;
#define WM_HR_SETTING_CHANGE   WM_USER + 0x105

// handyrun const
namespace hc
{
	extern const wchar_t * const empty_str;
	extern const wchar_t * const newline_str;
	extern const wchar_t * const handyrun_name;
	extern const wchar_t * const handyrun_wnd_title;
	extern const wchar_t * const fn_vkcode;
	extern const wchar_t * const fn_ini;
	extern const wchar_t * const err_exit;
	extern const wchar_t * const exe_ext;
};

