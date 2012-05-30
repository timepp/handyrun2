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

// hotkey�ṹ����һ���ȼ�(������ͨ�ȼ���˫���ȼ�)
struct hotkey
{
	UINT fk;        // ���ܼ�(control,alt,shift,win�����)
	                // ���fkΪ0,����һ��"˫��<vk>"���ȼ�
	UINT vk;        // ���̰�������
};

// ����Ļ�������(�������ļ��ṩ)
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

	bk_mode mode;                     // ����ģʽ(ͼƬ�������)
	color_list_t color_list;          // (��������)��ɫ�б�
	int fill_direction;               // ���Ƕȣ���0��360
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

