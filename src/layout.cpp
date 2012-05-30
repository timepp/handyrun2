#include "stdafx.h"
#include "layout.h"
#include "helper.h"
#include <limits>

using namespace lyt;

// gap-sequence: 有间距的序列
// |unit_len| gap |
// **********     **********     **********     **********
// |----------------gslen--------------------------------|
// 返回序列的总长度
static int gs_len(int unit_len, int gap, int unit_count)
{
	if (unit_count <= 0) return 0;
	return (unit_len + gap) * unit_count - gap;
}
// 返回给定的点落在哪个序列单元内部
static int gs_unit(int unit_len, int gap, int x)
{
	x += gap;
	int l = unit_len + gap;
	int r = x / l;
	if (x % l < gap) return -1;
	return r;
}
static int gs_area(int unit_len, int gap, int x)
{
	if (x < 0) return -1;
	return (x + gap) / (unit_len + gap);
}

// layout ///////////////////////////////////////////////////////////
#undef max
extern const lyt::prog_pos lyt::pp_null = {
	std::numeric_limits<int>::max(), 
	std::numeric_limits<int>::max()
};

bool prog_pos::operator <(const prog_pos& pp) const
{
	return g < pp.g || (g == pp.g && p < pp.p);
}

bool prog_pos::operator == (const prog_pos& pp) const
{
	return g == pp.g && p == pp.p;
}

bool prog_pos::operator != (const prog_pos& pp) const
{
	return !(*this == pp);
}

bool layout::init(HWND target_wnd) 
{
	m_target_wnd = target_wnd;
	return true; 
}
prog_pos layout::get_prog_from_point(POINT) const
{
	return pp_null;
}
RECT layout::get_prog_rect(prog_pos) const
{
	RECT rc = {0, 0, 0, 0};
	return rc;
}
prog_pos layout::get_insert_pos(POINT) const
{
	return pp_null;
}
void layout::draw_insert_mark(HDC , prog_pos, bool ) const
{
}
HICON layout::get_icon(prog_pos) const
{
	return NULL;
}
bool layout::draw_prog(HDC , prog_pos , prog_status ) const
{
	return true;
}
bool layout::draw_all_prog(HDC ) const
{
	return true;
}
void layout::move_prog_pos(prog_pos *, move_type) const
{
}


// trivial layout ///////////////////////////////////////////////
SIZE trivial_layout::get_window_size() const
{
	cfg::gui_metrics & gm = cfg::config::instance()->gm;
	SIZE sz = {gm.min_window_width, gm.border * 2 + gm.edit_height};
	return sz;
}

// icon_layout //////////////////////////////////////////////////
bool icon_layout::init(HWND target_wnd)
{
	layout::init(target_wnd);

	cfg::config * c = cfg::config::instance();
	// pdmap
	for (int i = 0; i < c->group_mgr.cs_count(); i++)
	{
		for (int j = 0; j < c->group_mgr.cmd_count(i); j++)
		{
			prog_pos pp = {i, j};
			prog_disp pd = {
				calc_prog_rect(pp),
				c->gm.cache_icon ? get_icon(pp) : 0
			};
			m_pdmap.insert(pdmap_t::value_type(pp, pd));
		}
	}

	return true;
}

icon_layout::~icon_layout()
{
	for (pdmap_t::const_iterator ci = m_pdmap.begin(); ci != m_pdmap.end(); ++ci)
	{
		if (ci->second.icon) ::DestroyIcon(ci->second.icon);
	}
}

RECT icon_layout::get_prog_rect(prog_pos pp) const
{
	pdmap_t::const_iterator ci = m_pdmap.find(pp);
	if (ci != m_pdmap.end())
	{
		return ci->second.rc;
	}
	RECT rc = {0, 0, 0, 0};
	return rc;
}

bool icon_layout::draw_all_prog(HDC hdc) const
{
	CDCHandle dc(hdc);
	for (pdmap_t::const_iterator ci = m_pdmap.begin(); ci != m_pdmap.end(); ++ci)
	{
		CRect rc(ci->second.rc);
		HICON icon = ci->second.icon ? ci->second.icon : get_icon(ci->first);
		dc.DrawIconEx(rc.left, rc.top, icon, rc.Width(), rc.Height());
		if (icon != ci->second.icon) ::DestroyIcon(icon);
	}
	return true;
}

void icon_layout::draw_insert_mark(HDC hdc, prog_pos pp, bool draw) const
{
	if (pp == pp_null) return;
	cfg::config * c = cfg::config::instance();
	int cmd_count = c->group_mgr.cmd_count(pp.g);
	// TODO: 当组内程序为空时也要画!
	if (cmd_count == 0) return;
	
	bool mark_left = true;
	if (pp.p >= cmd_count)
	{
		mark_left = false;
		pp.p = cmd_count - 1;
	}

	cfg::gui_metrics &m = c->gm;
	RECT rc = m_pdmap.find(pp)->second.rc;
	if (mark_left)
	{
		rc.left -= m.pad_h / 2;
	}
	else
	{
		rc.left = rc.right + m.pad_h / 2;
	}
	rc.right = rc.left + 2;
	CDCHandle dc(hdc);
	if (draw)
	{
		dc.FillRect(&rc, COLOR_HOTLIGHT);
	}
	else
	{
		::RedrawWindow(m_target_wnd, &rc, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW);
	}
}

bool icon_layout::draw_prog(HDC hdc, prog_pos pp, lyt::prog_status ps) const
{
	if (pp == pp_null) return false;

	prog_disp pd = m_pdmap.find(pp)->second;
	CRect rc = pd.rc;
	rc.InflateRect(3, 3);

	CDCHandle dc(hdc);
	if (ps == normal)
	{
		::RedrawWindow(m_target_wnd, &rc, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW);
		rc.DeflateRect(3, 3);
		HICON icon = pd.icon? pd.icon : get_icon(pp);
		dc.DrawIconEx(rc.left, rc.top, icon, rc.Width(), rc.Height());
		if (icon != pd.icon) ::DestroyIcon(icon);
	}
	else if (ps == active)
	{
		dc.DrawEdge(rc, EDGE_RAISED, BF_RECT);
	}
	else if (ps == pressed)
	{
		dc.DrawEdge(rc, EDGE_SUNKEN, BF_RECT);
	}
	else if (ps == highlight)
	{
	}
	else
	{
	}
	return true;
}

HICON icon_layout::get_icon(prog_pos pp) const
{
	const prog * p = cfg::config::instance()->get_prog(pp.g, pp.p);
	if (p == NULL) return NULL;

	std::wstring icon_file = p->icon.file;
	if (icon_file.empty()) icon_file = p->path;
	icon_file = hlp::abs_path(icon_file.c_str());
	HICON icon = ::ExtractIcon(NULL, icon_file.c_str(), p->icon.index);
	if (icon == NULL)
	{
		icon = hlp::get_file_icon(icon_file.c_str());
	}
	return icon;
}

bool aligned_icon_layout::init(HWND target_wnd)
{
	m_pp2rc.clear();
	m_rc2pp.clear();
	m_rc2ip.clear();
	return 
		init_maps() &&
		icon_layout::init(target_wnd);
}

SIZE aligned_icon_layout::get_window_size() const
{
	int row, col;
	cfg::gui_metrics & gm = cfg::config::instance()->gm;
	get_max_row_col(&row, &col);
	SIZE sz = 
	{
		gm.border * 2 + gs_len(gm.icon_size, gm.pad_h, col),
		gm.border * 2 + gs_len(gm.icon_size, gm.pad_v, row)
	};
	if (sz.cx < gm.min_window_width) sz.cx = gm.min_window_width;
	if (sz.cy < 50)                  sz.cy = 50;
	return sz;
}

RECT aligned_icon_layout::calc_prog_rect(prog_pos pp) const
{
	pp2rc_map_t::const_iterator ci = m_pp2rc.find(pp);
	rowcol r = ci->second;
	cfg::gui_metrics &m = cfg::config::instance()->gm;
	RECT rc;
	rc.left = m.border + (m.icon_size + m.pad_h) * r.second;
	rc.top  = m.border + (m.icon_size + m.pad_v) * r.first;
	rc.right = rc.left + m.icon_size;
	rc.bottom = rc.top + m.icon_size;
	return rc;
}

prog_pos aligned_icon_layout::get_prog_from_point(POINT pt) const
{
	cfg::gui_metrics &m = cfg::config::instance()->gm;
	pt.x -= m.border;
	pt.y -= m.border;
	int row = gs_unit(m.icon_size, m.pad_v, pt.y);
	int col = gs_unit(m.icon_size, m.pad_h, pt.x);
	if (row < 0 || col < 0) return pp_null;
	rowcol r(row, col);
	rc2pp_map_t::const_iterator ci = m_rc2pp.find(r);
	if (ci == m_rc2pp.end()) return pp_null;
	prog_pos pp = ci->second;
	if (!cfg::config::instance()->get_prog(pp.g, pp.p)) return pp_null;
	return pp;
}

prog_pos aligned_icon_layout::get_insert_pos(POINT pt) const
{
	cfg::gui_metrics &m = cfg::config::instance()->gm;
	pt.x -= m.border;
	pt.y -= m.border;
	int row = gs_unit(m.icon_size, m.pad_v, pt.y);
	int col = gs_area(m.icon_size, m.pad_h, pt.x);
	if (row < 0 || col < 0) return pp_null;
	
	rc2pp_map_t::const_iterator ci = m_rc2ip.end();
	while (col >= 0 && ci == m_rc2ip.end())
	{
		rowcol r(row, col--);
		ci = m_rc2ip.find(r);
	}
	return (ci == m_rc2ip.end()? pp_null : ci->second);
}

void aligned_icon_layout::move_prog_pos(prog_pos *pp, lyt::move_type mt) const
{
	cfg::config * c = cfg::config::instance();
	if (m_rc2pp.size() == 0) return;
	if (!c->get_prog(pp->g, pp->p))
	{
		pp->g = pp->p = 0;
		return;
	}

	rowcol r(0, 0);
	{
		pp2rc_map_t::const_iterator ci = m_pp2rc.find(*pp);
		if (ci != m_pp2rc.end()) r = ci->second;
	}
	
	int row_max, col_max;
	get_max_row_col(&row_max, &col_max);
	int &row = r.first;
	int &col = r.second;
	bool need_travel = true;
	std::pair<int, int> dir;
	switch (mt)
	{
	case mt_left:  dir = std::make_pair(0, -1);        break;
	case mt_right: dir = std::make_pair(0,  1);         break;
	case mt_up:    dir = std::make_pair(-1, 0);         break;
	case mt_down:  dir = std::make_pair( 1, 0);         break;
	case mt_home:  col = -1;      dir = std::make_pair(0,  1); break;
	case mt_end:   col = col_max; dir = std::make_pair(0, -1); break;

	case mt_head:  row = col = 0; need_travel = false; break;
	case mt_tail:  r = m_rc2pp.rbegin()->first; need_travel = false; break;
	}
	if (need_travel)
	{
		row += dir.first; col += dir.second;
		while (row >= 0 && col >= 0 && row < row_max && col < col_max)
		{
			if (m_rc2pp.find(r) != m_rc2pp.end()) break;
			row += dir.first; col += dir.second;
		}
	}
	rc2pp_map_t::const_iterator ci = m_rc2pp.find(r);
	if (ci != m_rc2pp.end())
	{
		*pp = ci->second;
	} 
}

bool group_icon_layout::init_maps()
{
	cfg::config * c = cfg::config::instance();
	rowcol r;
	prog_pos pp;
	for (int i = 0; i < c->group_mgr.cs_count(); i++)
	{
		for (int j = 0; j <= c->group_mgr.cmd_count(i); j++)
		{
			r.first = i; r.second = j;
			pp.g = i; pp.p = j;
			m_rc2ip[r] = pp;
			if (j < c->group_mgr.cmd_count(i))
			{
				m_pp2rc[pp] = r;
				m_rc2pp[r] = pp;
			}
		}
	}

	return true;
}

SIZE group_icon_layout::get_window_size() const
{
	SIZE sz = aligned_icon_layout::get_window_size();
	sz.cy += 6;
	return sz;
}

void group_icon_layout::get_max_row_col(int *row, int *col) const
{
	cfg::config * c = cfg::config::instance();
	*row = c->group_mgr.cs_count();
	*col = 0;
	for (int i = 0; i < *row; i++)
	{
		int sz = c->group_mgr.cmd_count(i);
		if (*col < sz) *col = sz;
	}
}

bool group_icon_layout::draw_all_prog(HDC hdc) const
{
	draw_group_line(hdc);
	return icon_layout::draw_all_prog(hdc);
}

// 画group间的分隔线
void group_icon_layout::draw_group_line(HDC hdc) const
{
	cfg::config * c = cfg::config::instance();
	for (int i = 0; i < c->group_mgr.cs_count(); i++)
	{
		const group_info &gi = *c->get_group_info(i);
		const cfg::gui_metrics& gm = c->gm;
		int y = gm.border + gm.icon_size * (i+1) + gm.pad_v * i +
			min(5, gm.pad_v / 2);

		if ((gi.color & 0xFF000000) == 0)
		{
			int x1 = gm.border;
			int x2 = gm.border + (gm.icon_size + gm.pad_h) * c->group_mgr.cmd_count(i);
			if (x2 <= x1) x2 = x1 + gm.icon_size;
			COLORREF cr = gi.color;
			hlp::draw_line(hdc, x1, y, x2, y, cr);
			cr = hlp::mid_color(cr, GetSysColor(COLOR_3DFACE), 0.8);
			hlp::draw_line(hdc, x1, y+1, x2, y+1, cr);
		}
	}
}

bool plain_icon_layout::init_maps()
{
	cfg::config * c = cfg::config::instance();
	rowcol r(0, 0);
	prog_pos pp;
	int i = -1, j = -1;
	for (i = 0; i < c->group_mgr.cs_count(); i++)
	{
		for (j = 0; j < c->group_mgr.cmd_count(i); j++)
		{
			if (r.second == c->gm.icon_per_line)
			{
				r.first++;
				r.second = 0;
			}
			pp.g = i; pp.p = j;
			m_pp2rc[pp] = r;
			m_rc2pp[r] = pp;
			m_rc2ip[r] = pp;
			r.second++;
		}
	}

	r.second++;
	pp.g = i-1; pp.p = j;
	m_rc2ip[r] = pp;

	return true;
}
void plain_icon_layout::get_max_row_col(int *row, int *col) const
{
	cfg::config * c = cfg::config::instance();
	int total = c->group_mgr.cmd_count();
	int p = c->gm.icon_per_line;
	int a = total / p;
	int b = total % p;
	*col = (a == 0? total : p);
	*row = (b == 0? a : a + 1);
}
