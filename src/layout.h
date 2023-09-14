#pragma once

#include "prog.h"
#include "config.h"
#include <windows.h>
#include <map>
#include <utility>

namespace lyt
{
	struct prog_pos
	{
		int g;                               // 组位置
		int p;                               // 程序位置
		bool operator < (const prog_pos& pp) const;
		bool operator == (const prog_pos& pp) const;
		bool operator != (const prog_pos& pp) const;
	};

	extern const prog_pos pp_null;

	enum prog_status { normal, active, pressed, highlight };
	enum move_type {mt_left, mt_right, mt_up, mt_down, mt_home, mt_end, mt_head, mt_tail, mt_none};

// 程序显示的抽象接口
class layout
{
public:
	virtual ~layout(){}
	// 初始化
	virtual bool init(HWND target_wnd);

	// 得到窗口客户区大小
	virtual SIZE get_window_size() const = 0;
	// 根据鼠标坐标获取程序，失败返回pp_null
	virtual prog_pos get_prog_from_point(POINT pt) const;
	// 得到插入点，失败返回pp_null
	virtual prog_pos get_insert_pos(POINT pt) const;
	// 取得指定程序的图标
	virtual HICON get_icon(prog_pos pp) const;
	// 取得prog的RECT
	virtual RECT get_prog_rect(prog_pos pp) const;

	// 用指定的样式画程序
	virtual bool draw_prog(HDC hdc, prog_pos pp, prog_status ps) const;
	// 画所有程序
	virtual bool draw_all_prog(HDC hdc) const;
	// 画插入点
	virtual void draw_insert_mark(HDC hdc, prog_pos pp, bool draw) const;
	// TODO： 得到物理上相邻的程序
	virtual void move_prog_pos(prog_pos *pp, move_type mt) const;

protected:
	HWND m_target_wnd;
};

class trivial_layout : public layout
{
public:
	virtual SIZE get_window_size() const;
};

// icon_layout: 用icon来表示程序
class icon_layout : public layout
{
public:
	~icon_layout();
	virtual bool init(HWND target_wnd);
	
	virtual HICON get_icon(prog_pos pp) const;
	virtual void draw_insert_mark(HDC hdc, prog_pos pp, bool draw) const;
	virtual bool draw_prog(HDC hdc, prog_pos pp, prog_status ps) const;
	virtual bool draw_all_prog(HDC hdc) const;
	virtual RECT get_prog_rect(prog_pos pp) const;
private:
	virtual RECT calc_prog_rect(prog_pos pp) const = 0;
	struct prog_disp
	{
		RECT rc;
		HICON icon;
	};
	typedef std::map<prog_pos, prog_disp> pdmap_t;
	pdmap_t m_pdmap;
};

// 图标大小一致，且排列整齐
class aligned_icon_layout : public icon_layout
{
public:
	virtual bool init(HWND target_wnd);

	virtual RECT calc_prog_rect(prog_pos pp) const;
	virtual SIZE get_window_size() const;
	virtual prog_pos get_prog_from_point(POINT pt) const;
	virtual prog_pos get_insert_pos(POINT pt) const;
	virtual void move_prog_pos(prog_pos *pp, move_type mt) const;
protected:
	typedef std::pair<int, int> rowcol;
	typedef std::map<rowcol, prog_pos> rc2pp_map_t;
	typedef std::map<prog_pos, rowcol> pp2rc_map_t;
	rc2pp_map_t m_rc2pp;  // 通过row, col找prog pos
	rc2pp_map_t m_rc2ip;  // 通过row, col找insert pos
	pp2rc_map_t m_pp2rc;  // 通过prog pos找row, col
private:
	// 返回最大的行、列值
	virtual void get_max_row_col(int * row, int *col) const = 0;
	// 初始化所有的map
	virtual bool init_maps() = 0;
};

class group_icon_layout : public aligned_icon_layout
{
public:
	virtual bool draw_all_prog(HDC hdc) const;
	virtual SIZE get_window_size() const;

private:
	virtual void get_max_row_col(int * row, int *col) const;
	virtual bool init_maps();
	void draw_group_line(HDC hdc) const;
};

class plain_icon_layout : public aligned_icon_layout
{
private:
	virtual void get_max_row_col(int * row, int *col) const;
	virtual bool init_maps();
};

}
