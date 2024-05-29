#pragma once
#include <windows.h>
#include <string>

namespace hlp
{
// string manipulation ////////////////////////////////////
	bool is_space(wchar_t c);

	// match with wild char "?" and "*"
	bool wild_match(const wchar_t *wild, const wchar_t *src);

	wchar_t * wcscpy_lower(wchar_t * d, int d_len, const wchar_t *s);
	const wchar_t * wcsistr(const wchar_t * s, const wchar_t * sub_s);

	int remove_char(std::wstring * str, wchar_t ch);
	std::wstring quote_string(const wchar_t * str, wchar_t q = L'\"');

	// 在一个以空格为分隔的字符串列表中查找一个字符串
	int match_str_list(const wchar_t * list, const wchar_t * s);

// system specific ////////////////////////////////////////
	std::wstring get_tmp_file_name(const wchar_t * ext = 0);
	bool file_exists(const wchar_t * fn);
	std::wstring abs_path(const wchar_t * p, bool file = true);
	std::wstring rela_path(const wchar_t * p);
	// check whether `hwnd' belongs to current process(thread)
	bool wnd_in_same_process(HWND hwnd);
	bool wnd_in_same_thread(HWND hwnd);
	// take	`wnd' to the foreground
	bool focus_window(HWND wnd);
	// simulate a mouse event at (x,y)
	bool send_mouse_input(int x, int y, DWORD flags);
	// 判断是不是DOS命令(包括控制台应用)　需要完整路径
	bool is_dos_command(const std::wstring& cmd);
	// wrappers
	bool track_mouse_event(DWORD flag, HWND wnd, DWORD hover_time = HOVER_DEFAULT);
	POINT get_cursor_pos(HWND wnd);
	bool is_key_down(int vk);
	RECT get_dlg_item_rect(HWND wnd, int id);

	void rect_ensure_inside(RECT *rc, RECT rc_out);
	enum edge_type {et_l, et_r, et_t, et_b};
	edge_type rect_dock(RECT *rc, RECT rc_out);

	int hex_val(wchar_t ch);
	int hex_val(const wchar_t * ch, int n);

	COLORREF mid_color(COLORREF cr1, COLORREF cr2, double r);
	void draw_line(HDC hdc, int x1, int y1, int x2, int y2, COLORREF cr);

	HICON get_file_icon(const wchar_t * fn);
	HICON CreateGrayscaleIcon(HICON hIcon);
	void print_err(const wchar_t * fmt, ...);
	void print_sys_err(const wchar_t * info);
	void print_com_error(const wchar_t * info, HRESULT hr);

	struct path_elem
	{
		struct { int s, n; } path, name, ext;
	};
	path_elem split_path(const wchar_t * path);

	// TODO： 太难使用，去掉！！！
	size_t strcat_ex(wchar_t * buf, size_t buf_len, size_t pos, const wchar_t * src, size_t src_len = -1);

	void show_err(const wchar_t *err, const wchar_t *result);

	std::wstring expand_envvar(const wchar_t * str);
	std::wstring path_resolve(const wchar_t * str);

	HRESULT ShellExecuteByExplorer(const wchar_t* path, const wchar_t* param, const wchar_t* dir);

}
