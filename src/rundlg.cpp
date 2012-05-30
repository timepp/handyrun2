#include "stdafx.h"
#include "RunDlg.h"
#include "helper.h"
#include "defs.h"
#include <shellapi.h>
#include <math.h>
#include <string>
#include <algorithm>
#include "progdlg.h"
#include "importdlg.h"
#include "layout.h"
#include "error.h"
#include "log.h"
#include "hotkey_waker.h"
#include "global_data.h"
#include "inputdlg.h"
#include "groupdlg.h"
#include "imagedc.h"
#include "geometry.h"
#include <tplib/util_win.h>
#include <Shlwapi.h>

#undef min
#undef max
using std::min;
using std::max;

#define IDT_AUTO_FOLD 1002
#define IDT_AUTO_FOLD_WHEN_MOUSE_UP 1003
#define IDT_SAVE_CONFIG 1004
#define IDT_KEEPTOP 1005

static CRunDlg * g_dlg = NULL;
static cfg::config * g_c = NULL;

// op_state /////////////////////////////////////////////////////

void CRunDlg::hover_state::init()
{
	m_last_mouse_pos.x = -1;
	if (g_dlg->IsWindow() && g_dlg->can_fold())
	{
		g_dlg->SetTimer(IDT_AUTO_FOLD, 500);
	}
}
void CRunDlg::hover_state::cleanup()
{
	if (g_c->gm.style == cfg::ws_edge && g_dlg->IsWindow()) 
		g_dlg->KillTimer(IDT_AUTO_FOLD);
}
void CRunDlg::hover_state::handle_mouse(UINT msg, WPARAM , POINT pt)
{
	if (msg == WM_MOUSEMOVE)
	{
		if ((CPoint)m_last_mouse_pos == pt) return;
		if (m_last_mouse_pos.x == -1)
		{
			m_last_mouse_pos = pt;
			return;
		}
	}

	lyt::layout * l = g_dlg->m_lyt;
	prog_pos old_pp = l->get_prog_from_point(m_last_mouse_pos);
	m_last_mouse_pos = pt;
	prog_pos pp = l->get_prog_from_point(pt);
	if (old_pp != pp || g_dlg->m_curr_prog_pos != pp)
	{
		CClientDC dc(g_dlg->m_hWnd);
		l->draw_prog(dc, old_pp, lyt::normal);
		l->draw_prog(dc, g_dlg->m_curr_prog_pos, lyt::normal);
		l->draw_prog(dc, pp, lyt::active);
		g_dlg->SetNewTip();
		g_dlg->m_curr_prog_pos = pp;
	}

	if (msg == WM_MOUSEMOVE)
	{
		hlp::track_mouse_event(TME_LEAVE, g_dlg->m_hWnd);
	}
	if (msg == WM_LBUTTONDOWN)
	{
		hlp::track_mouse_event(TME_CANCEL|TME_LEAVE, g_dlg->m_hWnd);
		if (pp != pp_null)
		{
			l->draw_prog(CClientDC(g_dlg->m_hWnd), pp, lyt::pressed);
			g_dlg->change_op_state(os_run);
		}
		else
		{
			g_dlg->change_op_state(os_drag_wnd);
		}
	}
}

void CRunDlg::run_state::init()
{
	m_pressed_pos = hlp::get_cursor_pos(g_dlg->m_hWnd);
	m_pressed_pp = g_dlg->m_lyt->get_prog_from_point(m_pressed_pos);
}
void CRunDlg::run_state::cleanup()
{
	g_dlg->m_lyt->draw_prog(CClientDC(g_dlg->m_hWnd), m_pressed_pp, lyt::normal);
}
void CRunDlg::run_state::handle_mouse(UINT msg, WPARAM , POINT pt)
{
	lyt::layout * l = g_dlg->m_lyt;
	prog_pos pp = l->get_prog_from_point(pt);

	if (msg == WM_MOUSEMOVE)
	{
		if (abs(pt.x - m_pressed_pos.x) + abs(pt.y - m_pressed_pos.y) > 5)
		{
			l->draw_prog(CClientDC(g_dlg->m_hWnd), m_pressed_pp, lyt::normal);
			g_dlg->change_op_state(os_drag_prog);
		}
	}
	if (msg == WM_LBUTTONUP)
	{
		if (pp != pp_null && pp == m_pressed_pp)
		{
			g_dlg->OnClickProg(g_c->get_prog(pp.g, pp.p));
		}
		l->draw_prog(CClientDC(g_dlg->m_hWnd), m_pressed_pp, lyt::normal);
		g_dlg->change_op_state(os_hover);
	}
}
void CRunDlg::drag_window_state::init()
{
	m_pressed_pos = hlp::get_cursor_pos(g_dlg->m_hWnd);
	g_dlg->SetCapture();
}
void CRunDlg::drag_window_state::cleanup()
{
	ReleaseCapture();
}
void CRunDlg::drag_window_state::handle_mouse(UINT msg, WPARAM , POINT pt)
{
	if (msg == WM_MOUSEMOVE)
	{
		CRect rc;
		g_dlg->GetWindowRect(&rc);
		rc.OffsetRect(pt.x - m_pressed_pos.x, pt.y - m_pressed_pos.y);
		cfg::gui_metrics &g = g_c->gm;
		if (g.style == cfg::ws_edge)
		{
			RECT rc_screen = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
			switch (hlp::rect_dock(&rc, rc_screen))
			{
			case hlp::et_t: g.edge = cfg::ec_t; g.edge_pos = rc.left; break;
			case hlp::et_l: g.edge = cfg::ec_l; g.edge_pos = rc.top;  break;
			case hlp::et_r: g.edge = cfg::ec_r; g.edge_pos = rc.top;  break;
			case hlp::et_b: g.edge = cfg::ec_b; g.edge_pos = rc.left; break;
			}
		}
		g_dlg->MoveWindow(&rc);
	}
	if (msg == WM_LBUTTONUP)
	{
		g_dlg->change_op_state(os_hover);
	}
}
void CRunDlg::drag_prog_state::init()
{
	m_last_mouse_pos = hlp::get_cursor_pos(g_dlg->m_hWnd);
	m_dragged_pp = g_dlg->m_curr_prog_pos;
	cfg::gui_metrics& m = g_c->gm;
	m_il.Create(m.icon_size, m.icon_size, ILC_COLOR32 | ILC_MASK, 1, 1);
	m_il.SetBkColor(CLR_NONE);
	HICON icon = g_dlg->m_lyt->get_icon(m_dragged_pp);
	m_il.AddIcon(icon);
	::DestroyIcon(icon);
	m_il.BeginDrag(0, 0, 0);
	m_il.DragEnter(g_dlg->m_hWnd, m_last_mouse_pos);
	g_dlg->SetCapture();
}
void CRunDlg::drag_prog_state::cleanup()
{
	prog_pos old_pp = g_dlg->m_lyt->get_insert_pos(m_last_mouse_pos);
	g_dlg->m_lyt->draw_insert_mark(CClientDC(g_dlg->m_hWnd), old_pp, false);
	m_il.DragLeave(g_dlg->m_hWnd);
	m_il.EndDrag();
	m_il.Destroy();
	::ReleaseCapture();
}
void CRunDlg::drag_prog_state::handle_mouse(UINT msg, WPARAM , POINT pt)
{
	m_il.DragMove(pt);
//
	lyt::layout * l = g_dlg->m_lyt;
	prog_pos pp, old_pp;
	old_pp = l->get_insert_pos(m_last_mouse_pos);
	m_last_mouse_pos = pt;
	pp = l->get_insert_pos(pt);
	if (pp != old_pp)
	{
		m_il.DragShowNolock(FALSE);
		HDC dc = g_dlg->GetDC();
		l->draw_insert_mark(dc, old_pp, false);
		l->draw_insert_mark(dc, pp, true);
		g_dlg->ReleaseDC(dc);
		m_il.DragShowNolock(TRUE);
	}

	if (msg == WM_LBUTTONUP)
	{
		m_il.DragLeave(g_dlg->m_hWnd);
		m_il.EndDrag();

		if (pp != pp_null)
		{
			prog_pos opp = m_dragged_pp;
			prog *p = g_c->get_prog(m_dragged_pp.g, m_dragged_pp.p);
			prog * new_p = new prog(*p);
			if (!hlp::is_key_down(VK_CONTROL))
			{
				g_c->group_mgr.delete_cmd(opp.g, opp.p);
				if (pp.g == opp.g && opp.p < pp.p) 
				{
					pp.p--;
				}
			}
			g_c->group_mgr.insert_cmd(new_p, pp.g, pp.p);
			
			g_dlg->refresh_layout();
			g_dlg->show_window(true);
		}
		g_dlg->change_op_state(os_hover);
	}
}

void CRunDlg::drop_target_state::init() {}
void CRunDlg::drop_target_state::cleanup() {}
void CRunDlg::drop_target_state::handle_mouse(UINT, WPARAM, POINT) {}
DWORD CRunDlg::drop_target_state::drag_enter(IDataObject *data, POINT pt)
{
	if (!g_dlg->m_current_show_state && g_c->gm.style == cfg::ws_edge)
	{
		// 显示窗口
		g_dlg->show_window(true);
	}
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if (data->QueryGetData(&fmtetc) == S_OK)
	{
		m_data_format = CF_HDROP;
	}
	else
	{
		fmtetc.cfFormat = CF_UNICODETEXT;
		m_data_format = (data->QueryGetData(&fmtetc) == S_OK)? CF_UNICODETEXT : 0;
	}

	m_last_mouse_pos.x = m_last_mouse_pos.y = -1;
	return drag_over(pt);
}

DWORD CRunDlg::drop_target_state::drag_over(POINT pt)
{
	if (g_c->gm.layout == cfg::ul_simple)
	{
		return (m_data_format == CF_UNICODETEXT) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
	}

	lyt::layout * l = g_dlg->m_lyt;
	prog_pos old_pp = l->get_prog_from_point(m_last_mouse_pos);
	prog_pos old_ins_pp = l->get_insert_pos(m_last_mouse_pos);
	prog_pos pp = l->get_prog_from_point(pt);
	prog_pos ins_pp = l->get_insert_pos(pt);
	m_last_mouse_pos = pt;
	CClientDC dc(g_dlg->m_hWnd);
	if (old_pp != pp) l->draw_prog(dc, old_pp, lyt::normal);
	if (old_ins_pp != ins_pp || pp != old_pp) 
		l->draw_insert_mark(dc, old_ins_pp, false);
	if (pp != pp_null) 
	{
		l->draw_prog(dc, pp, lyt::active);
	}
	else if (ins_pp != pp_null)
	{
		if (m_data_format != CF_HDROP) return DROPEFFECT_NONE;
		l->draw_insert_mark(dc, ins_pp, true);
	}
	else
	{
		return DROPEFFECT_NONE;
	}
	return DROPEFFECT_COPY;
}

void CRunDlg::drop_target_state::drag_leave()
{
	lyt::layout * l = g_dlg->m_lyt;
	prog_pos old_pp = l->get_prog_from_point(m_last_mouse_pos);
	prog_pos old_ins_pp = l->get_insert_pos(m_last_mouse_pos);
	CClientDC dc(g_dlg->m_hWnd);
	l->draw_prog(dc, old_pp, lyt::normal);
	l->draw_insert_mark(dc, old_ins_pp, false);
}

DWORD CRunDlg::drop_target_state::drop(IDataObject *data, POINT pt)
{
	if (g_c->gm.layout == cfg::ul_simple)
	{
		g_dlg->m_edit.ReplaceSel(get_drop_text(data).c_str());
		hlp::focus_window(g_dlg->m_hWnd);
		return DROPEFFECT_COPY;
	}
	
	lyt::layout * l = g_dlg->m_lyt;
	prog_pos pp = l->get_prog_from_point(pt);
	if (pp != pp_null)
	{
		// 用拖过来的文件名扩展播放参数运行程序
		// 如果拖过来的是文本，就存到临时文件中，并以此当做文件名
		prog * pg = g_c->get_prog(pp.g, pp.p);
		if (pg != g_dlg->m_prog_waiting_param)
		{
			g_dlg->m_prog_waiting_param = pg;
			m_cached_var.clear();
			m_var_count = get_param_var_count(pg->drop_param);
		}
		if (!get_drop_files(data, &m_cached_var))
		{
			std::wstring fn = hlp::get_tmp_file_name();
			FILE * fp;
			if (_wfopen_s(&fp, fn.c_str(), L"w, ccs=UTF-8") != 0)
			{
				os_err oe(dos, L"写文件", L"  文件名:%s", fn);
				hlp::show_err(oe.what(), 0);
			}
			else
			{
				fwprintf(fp, L"%s", get_drop_text(data).c_str());
				fclose(fp);
				m_cached_var.push_back(fn);
			}
		}
		if (m_cached_var.size() >= m_var_count)
		{
			prog tmp_p = *pg;
			if (tmp_p.drop_param.empty()) tmp_p.drop_param = tmp_p.param;
			tmp_p.param = expand_drop_param(tmp_p.drop_param);
			g_dlg->OnClickProg(&tmp_p);
		}
		else
		{
			g_dlg->show_window(false);
		}
	}
	else
	{
		pp = l->get_insert_pos(pt);
		if (pp != pp_null)
		{
			string_list_t sl;
			get_drop_files(data, &sl);
			string_list_t::const_reverse_iterator ci;
			for (ci = sl.rbegin(); ci != sl.rend(); ++ci)
			{
				prog * p = new prog;
				p->path = hlp::rela_path(ci->c_str());
				g_c->group_mgr.insert_cmd(p, pp.g, pp.p);
			}
			g_dlg->refresh_layout();
			g_dlg->show_window(true);
		}
	}
	
	drag_leave();
	return DROPEFFECT_COPY;
}

std::wstring CRunDlg::drop_target_state::get_drop_text(IDataObject *data)
{
	FORMATETC fmtetc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;
	std::wstring str;
	if (data->GetData(&fmtetc, &stgmed) == S_OK)
	{
		LPVOID buf = GlobalLock(stgmed.hGlobal);
		str = (LPCWSTR)buf;
		GlobalUnlock(stgmed.hGlobal);
		ReleaseStgMedium(&stgmed);
	}
	hlp::remove_char(&str, L'\r');
	return str;
}

bool CRunDlg::drop_target_state::get_drop_files(IDataObject *data, std::list<std::wstring> *fl)
{
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;
	if(data->GetData(&fmtetc, &stgmed) == S_OK)
	{
		wchar_t buf[MAX_PATH]=L"";
		HDROP hDrop = (HDROP)stgmed.hGlobal;
		UINT count = ::DragQueryFile(hDrop, 0xFFFFFFFF, buf, 0);
		for (UINT i = 0; i < count; i++)
		{
			if(::DragQueryFile(hDrop, i, buf, elem_of(buf)))
			{
				fl->push_back(buf);
			}
		}
		::DragFinish(hDrop);
		ReleaseStgMedium(&stgmed);
		return true;
	}
	return false;
}

int CRunDlg::drop_target_state::get_param_var_count(const std::wstring& param)
{
	int count = 0;
	for (size_t i = 0; i + 1 < param.length(); i++)
	{
		if (param[i] == L'%')
		{
			wchar_t ch = param[i+1];
			if (ch == L'%')
			{
				i++;
			}
			else if (ch >= L'0' && ch <= L'9')
			{
				if (count < ch - L'0') count = ch - L'0';
			}
		}
	}
	return count;
}

std::wstring CRunDlg::drop_target_state::expand_drop_param(const std::wstring& param) const
{
	std::wstring ret;
	int expand_count = 0;
	size_t i;
	for (i = 0; i + 1 < param.length(); i++)
	{
		if (param[i] == L'%')
		{
			wchar_t ch = param[i+1];
			if (ch == L'%')
			{
				ret.push_back(ch);
				i++;
			}
			else if (ch >= L'1' && ch <= L'9')
			{
				size_t index = ch - L'0';
				if (m_cached_var.size() > index) 
				{
					string_list_t::const_iterator ci = m_cached_var.begin();
					for (size_t j = 0; j < index; j++) ++ci;
					ret += hlp::quote_string(ci->c_str());
				}
				i++;
				expand_count++;
			}
			else
			{
				ret.push_back(ch);
			}
		}
		else
		{
			ret.push_back(param[i]);
		}
	}
	if (i < param.length()) ret.push_back(param[i]);
	if (expand_count == 0)
	{
		for (string_list_t::const_iterator ci = m_cached_var.begin(); ci != m_cached_var.end(); ++ci)
		{
			ret += L" " + hlp::quote_string(ci->c_str());
		}
	}
	return ret;
}

CRunDlg::CRunDlg() :
CRegionTipDlgImpl<CRunDlg>(cfg::config::instance()->gm.use_tip)
{
	m_lyt = NULL;
	m_background = NULL;
	g_c = cfg::config::instance();
	g_dlg = this;
	m_current_show_state = false;
	m_curr_prog_pos = pp_null;
	m_last_cmd_forced = false;
	m_prog_waiting_param = 0;
}

CRunDlg * CRunDlg::Instance()
{
	static CRunDlg dlg;
	return &dlg;
}

CRunDlg::~CRunDlg()
{
	delete m_lyt;
	m_lyt = NULL;
}

void CRunDlg::ui_exit()
{
	if (m_tip.IsWindow()) m_tip.DestroyWindow();
	if (IsWindow()) DestroyWindow();
	PostQuitMessage(0);
}

void CRunDlg::EndRequest()
{
	if (!gd::run_once)
	{
		show_window(false);
	}
	else
	{
		ui_exit();
	}
}

void CRunDlg::AdjustWndSize()
{
	SIZE sz = m_lyt->get_window_size();
	ResizeClient(sz.cx, sz.cy);
}

void CRunDlg::change_op_state(CRunDlg::ops os)
{
	op_state * new_os = NULL;
	switch (os)
	{
	case os_hover: new_os = &m_os_hover; break;
	case os_run:   new_os = &m_os_run; break;
	case os_drag_wnd: new_os = &m_os_drag_wnd; break;
	case os_drag_prog: new_os = &m_os_drag_prog; break;
	case os_drop_target: new_os = &m_os_drop_target; break;
	}
	if (m_os) m_os->cleanup();
	new_os->init();
	m_os = new_os;
}

void CRunDlg::refresh_layout()
{
	if (m_lyt) delete m_lyt;

	switch (cfg::config::instance()->gm.layout)
	{
	case cfg::ul_simple:  m_lyt = new lyt::trivial_layout(); break;
	case cfg::ul_plain:   m_lyt = new lyt::plain_icon_layout(); break;
	default:              m_lyt = new lyt::group_icon_layout(); break;
	}
	m_lyt->init(m_hWnd);
}

void CRunDlg::place_window_on_edge(bool show)
{
	cfg::gui_metrics& m = g_c->gm;
	CRect rc;
	GetWindowRect(&rc);
	int scr_w = ::GetSystemMetrics(SM_CXSCREEN);
	int scr_h = ::GetSystemMetrics(SM_CYSCREEN);
	int wnd_w = rc.Width();
	int wnd_h = rc.Height();
	int p = m.edge_pos;
	int l = m.edge_window_len;
	int x = 0, y = 0;
	if (show)
	{
		switch (m.edge)
		{
		case cfg::ec_t: x = p; y = 0; break;
		case cfg::ec_l: x = 0; y = p; break;
		case cfg::ec_b: x = p; y = scr_h; break;
		case cfg::ec_r: x = scr_w; y = p; break;
		}

		RECT rc_scr = {0, 0, scr_w, scr_h};
		RECT rc_wnd = {x, y, x+wnd_w, y+wnd_h};
		hlp::rect_dock(&rc_wnd, rc_scr);
		MoveWindow(&rc_wnd);
	}
	else
	{
		int ed = max(m.edge_window_thickness, 10);
		int el = ed - m.edge_window_thickness;
		int e_w = 0, e_h = 0;
		switch (m.edge)
		{
		case cfg::ec_t: x = p; y = -el; e_w = l; e_h = ed; break;
		case cfg::ec_l: x = -el; y = p; e_w = ed; e_h = l; break;
		case cfg::ec_b: x = p; y = scr_h + el; e_w = l; e_h = ed; break;
		case cfg::ec_r: x = scr_w + el; y = p; e_w = ed; e_h = l; break;
		}
		if (e_w == 0) e_w = wnd_w;
		if (e_h == 0) e_h = wnd_h;

		RECT rc_scr = {-el, -el, scr_w + el, scr_h + el};
		RECT rc_wnd = {x, y, x+e_w, y+e_h};
		hlp::rect_dock(&rc_wnd, rc_scr);
		ShowWindow(SW_HIDE);
		SetWindowPos(HWND_TOPMOST, &rc_wnd, SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
}

void CRunDlg::show_window(bool show, bool force_focus)
{
	m_current_show_state = show;
	if (m_tip.IsWindow() && m_tip.IsWindowVisible()) 
	{
		m_tip.ShowWindow(SW_HIDE);
	}
	if (!show && gd::run_once)
	{
		ui_exit();
		return;
	}

	if (show)
	{
		log_msg(L"activating window...");
		// 记录当前活动窗口备用
		HWND wnd = GetForegroundWindow();
		if (!hlp::wnd_in_same_process(wnd))
		{
			gd::prev_active_wnd = wnd;
		}

		AdjustWndSize();
		if (g_c->gm.style == cfg::ws_edge)
		{
			place_window_on_edge(true);
		}
		else
		{
			CenterWindow();
		}

		SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW|SWP_NOACTIVATE);
		if (force_focus)
		{
			BringWindowToTop();
			hlp::focus_window(m_hWnd);
		}
		RedrawWindow();
		m_curr_prog_pos = pp_null;
		change_op_state(os_hover);
		enable_type_command(g_c->gm.layout == cfg::ul_simple);
	}
	else
	{
		log_msg(L"deactivating window...");
		
		if (g_c->gm.style == cfg::ws_center)
		{
			ShowWindow(SW_HIDE);
		}
		else
		{
			place_window_on_edge(false);
		}
		change_op_state(os_hover);
		//ReleaseCapture();
		SetProcessWorkingSetSize(::GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
	}
}

// 执行程序
// 当shift按下时，先选择用户再运行
// 当alt按下时，以如下方式运行： cmd.exe /C <prog> &&pause
bool CRunDlg::DoExecute(const command * cmd)
{
	m_prog_waiting_param = 0;
	if (cmd == NULL) return false;

	std::wstring verb = hlp::is_key_down(VK_SHIFT) ? L"runas" : L"open";
	std::wstring path = hlp::abs_path(cmd->path.c_str());
	std::wstring param;
	std::wstring work_dir;
	int show_cmd = SW_SHOW;

	const prog *p;
	if ((p = dynamic_cast<const prog *>(cmd)) != NULL)
	{
		param = p->param;
		work_dir = p->work_dir;
		show_cmd = p->show_cmd;
	}

	if (work_dir.empty())
	{
		hlp::path_elem pe = hlp::split_path(path.c_str());
		work_dir.assign(path, pe.path.s, pe.path.n);
	}

	// alt键按下
	if (hlp::is_key_down(VK_MENU) ||
			g_c->go.keep_dos_cmd_window &&
			hlp::is_dos_command(path)
	   )
	{
		param = L"/C " + path + L" " + param + L"&pause";
		path = hlp::expand_envvar(L"%ComSpec%");
		work_dir = hc::empty_str;
	}

	if (verb == L"open")
	{
		if (tp::oswin::get_osverion(NULL) >= 6 &&
			tp::oswin::is_elevated())
		{
			HRESULT hr = hlp::ShellExecuteByExplorer(path.c_str(), param.c_str(), work_dir.c_str());
			if (FAILED(hr))
			{
				hlp::print_com_error(L"启动程序失败：", hr);
				return false;
			}
			return true;
		}
	}

	if (ShellExecute(m_hWnd, verb.c_str(), path.c_str(), param.c_str(), work_dir.c_str(), show_cmd) <= (HINSTANCE)32)
	{
		hlp::print_sys_err(L"启动程序失败：");
		return false;
	}

	return true;
}

void CRunDlg::reload_profile(void (*pf_modify_op)())
{
	hotkey_waker::instance()->register_hotkey(NULL);
	g_c->save();
	//ModifyStyleEx(WS_EX_TOOLWINDOW, 0);
	// 暂时取消总在最前属性
	SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

	pf_modify_op();

	// 恢复总在最前属性
	SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	//ModifyStyleEx(0, WS_EX_TOOLWINDOW);
	g_c->load();
	hotkey_waker::instance()->register_hotkey(g_c->go.active_key.c_str());

	refresh_layout();
	SetBackground(cfg::config::instance()->bk);
	show_window(true);
}

// 参数为true时显示输入框
// 参数为false时隐藏输入框，对话框窗口接收所有键盘消息
void CRunDlg::enable_type_command(bool enable)
{
	if (!hlp::wnd_in_same_process(::GetForegroundWindow())) return;

	if (enable)
	{
		m_edit.ShowWindow(SW_SHOW);
		m_edit.EnableWindow(TRUE);
		m_edit.SetFocus();
		m_edit.SetSel(0, -1);
		if (m_edit.GetWindowTextLengthW() > 0 && !m_last_cmd_forced)
		{
			m_tip.OnInputChange();
		}
	}
	else
	{
		m_edit.ShowWindow(SW_HIDE);
		m_edit.EnableWindow(FALSE);
		SetFocus();
	}
}

bool CRunDlg::can_fold() const
{
	return 
		m_current_show_state == true &&
		!hlp::wnd_in_same_process(::GetForegroundWindow())
		;  
}

LRESULT CRunDlg::OnEndSession(UINT, WPARAM, LPARAM, BOOL&)
{
	g_c->save();
	return 0;
}

LRESULT CRunDlg::OnShow(UINT, WPARAM , LPARAM, BOOL&)
{
	bool left_mouse_button_down = hlp::is_key_down(VK_LBUTTON);
	show_window(!m_current_show_state, !left_mouse_button_down);

	// 键盘激活且激活时鼠标按下：当鼠标抬起且不在handyrun窗口时，handyrun仍隐藏
	if (left_mouse_button_down)
	{
		SetTimer(IDT_AUTO_FOLD_WHEN_MOUSE_UP, 500);
	}

	// 键盘激活的窗口不自动隐藏
	if (m_current_show_state)
	{
		KillTimer(IDT_AUTO_FOLD);
	}
	return 0;
}

LRESULT CRunDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	log_msg(L"initializing main dialog...");

	SetBackground(cfg::config::instance()->bk);

	m_edit = GetDlgItem(IDC_PROG_EDIT);
	m_tip.Create(m_hWnd);
	m_tip.SetPair(GetDlgItem(IDC_PROG_EDIT));

	SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);

//	m_background  = (HBITMAP)::LoadImage(NULL, L"d:\\anyplace\\aa.BMP", IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);


	SetWindowText(L"HandyRun");
	refresh_layout();

	// 初始化DropTarget
	// 因为消息循环中收不到WM_CREATE消息，所以要手动的调用IDropTargetImpl中的OnCreate
	IDropTargetImpl<CRunDlg>::InitDropTarget();

	if (g_c->gm.style == cfg::ws_edge && !gd::run_once)
	{
		// TODO： 找窗口一开始不激活的更好的简决办法
		show_window(false);
		m_current_show_state = true;
		PostMessage(WM_SHOW_RUNDLG);
	}
	
	change_op_state(os_hover);

	::DragAcceptFiles(m_hWnd, TRUE);

	// 在vista下设置低权限程序可以发送来的消息类型
	HMODULE hm = ::LoadLibrary(L"user32.dll");
	if (hm)
	{
		typedef BOOL (WINAPI *CWMF_FuncPtr)(UINT message,DWORD dwFlag);
#pragma warning(push)
#pragma warning(disable: 4191)
		CWMF_FuncPtr proc = (CWMF_FuncPtr)::GetProcAddress(hm,"ChangeWindowMessageFilter");
#pragma warning(pop)
		if (proc)
		{
#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif
			for (UINT i = 0; i < 0x1000; i++)
			{
				(*proc)(i, MSGFLT_ADD);
			}
			(*proc)(WM_DROPFILES, MSGFLT_ADD);
(*proc)(WM_COPYDATA, MSGFLT_ADD);
(*proc)(0x0049, MSGFLT_ADD);
			(*proc)(WM_DROPFILES, MSGFLT_ADD);
			(*proc)(WM_MOUSEMOVE, MSGFLT_ADD);
			(*proc)(WM_NCMOUSEMOVE, MSGFLT_ADD);
		}
		::FreeLibrary(hm);
	}

	SetTimer(IDT_SAVE_CONFIG, 1000 * 600);
	SetTimer(IDT_KEEPTOP, 1000);
	return TRUE;
}

LRESULT CRunDlg::OnTimer(UINT, WPARAM wp, LPARAM, BOOL&)
{
	if (wp == IDT_AUTO_FOLD)
	{
		if (!can_fold())
		{
			KillTimer(wp);
			return 0;
		}
		POINT pt;
		GetCursorPos(&pt);
		if (!hlp::wnd_in_same_process(WindowFromPoint(pt)))
		{
			KillTimer(wp);
			EndRequest();
		}
	}
	else if (wp == IDT_AUTO_FOLD_WHEN_MOUSE_UP)
	{
		if (!can_fold())
		{
			KillTimer(wp);
			return 0;
		}
		POINT pt;
		GetCursorPos(&pt);
		if (!hlp::is_key_down(VK_LBUTTON) && !hlp::wnd_in_same_process(WindowFromPoint(pt)))
		{
			KillTimer(wp);
			EndRequest();
		}
	}
	else if (wp == IDT_SAVE_CONFIG)
	{
		g_c->save();
	}
	else if (wp == IDT_KEEPTOP)
	{
		if (g_c->gm.style == cfg::ws_edge && !m_current_show_state)
		{
			//show_window(false, false);
			::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}
	}
	return 0;
}

LRESULT CRunDlg::OnActivateApp(UINT , WPARAM wParam, LPARAM , BOOL& )
{
	if (wParam == FALSE && m_current_show_state &&
		g_c->gm.hide_on_lose_focus &&
		!hlp::is_key_down(VK_CONTROL))
	{
		EndRequest();
	}

	return TRUE;
}

LRESULT CRunDlg::OnCancel(WORD , WORD , HWND , BOOL& )
{
	if (m_os != &m_os_hover)
	{
		change_op_state(os_hover);
	}
	else if (m_tip.IsWindowVisible())
	{
		m_tip.ShowWindow(SW_HIDE);
	}
	else if (m_edit.IsWindowVisible() && g_c->gm.layout != cfg::ul_simple)
	{
		enable_type_command(false);
	}
	else
	{
		EndRequest();
	}
	return TRUE;
}

LRESULT CRunDlg::OnOK(WORD , WORD , HWND , BOOL &)
{
	return TRUE;
}

LRESULT CRunDlg::OnClose(UINT , WPARAM , LPARAM , BOOL& )
{
	ui_exit();
	return TRUE;
}

LRESULT CRunDlg::OnPaint(UINT , WPARAM , LPARAM , BOOL& )
{
	CPaintDC dc(m_hWnd);
	if (m_current_show_state)
	{
		m_lyt->draw_all_prog(dc);
	}
	return TRUE;
}

LRESULT CRunDlg::OnSize(UINT , WPARAM , LPARAM lParam, BOOL &)
{
	int cx = LOWORD(lParam);
	int cy = HIWORD(lParam);
	cfg::gui_metrics &g = g_c->gm;
	if (m_edit.IsWindow())
	{
		int flag = (g.layout == cfg::ul_simple)? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
		m_edit.SetWindowPos(0, g.border, cy - g.border - g.edit_height, cx - g.border * 2, g.edit_height, flag);
	}
	return TRUE;
}

LRESULT CRunDlg::OnNcMouseAction(UINT uMsg, WPARAM , LPARAM , BOOL& )
{
	if (g_c->gm.style == cfg::ws_edge &&
		(uMsg == WM_NCMOUSEMOVE || uMsg == WM_MOUSEMOVE) &&
		!m_current_show_state)
	{
		bool forceFocus = g_c->gm.layout == cfg::ul_simple;
		show_window(true, forceFocus);
	}
	return 0;
}

LRESULT CRunDlg::OnMouseAction(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OnNcMouseAction(uMsg, wParam, lParam, bHandled);
	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	if (uMsg == WM_MOUSELEAVE)
	{
		::GetCursorPos(&pt);
		::ScreenToClient(m_hWnd, &pt);
	}
	m_os->handle_mouse(uMsg, wParam, pt);
	return TRUE;
}

LRESULT CRunDlg::OnChar(UINT uMsg, WPARAM wp, LPARAM lp, BOOL&)
{
	if (wp >= 32)
	{
		enable_type_command(true);
		m_edit.SendMessageW(uMsg, wp, lp);
	}
	return 0;
}

LRESULT CRunDlg::OnKeyDown(UINT, WPARAM wp, LPARAM, BOOL&)
{
	prog_pos pp = m_curr_prog_pos;
	lyt::move_type mt = lyt::mt_none;
	bool control_pressed = hlp::is_key_down(VK_CONTROL);
	switch (wp)
	{
	case VK_LEFT:  mt = lyt::mt_left; break;
	case VK_RIGHT: mt = lyt::mt_right; break;
	case VK_UP:    mt = lyt::mt_up; break;
	case VK_DOWN:  mt = lyt::mt_down; break;
	case VK_HOME:  mt = control_pressed? lyt::mt_head : lyt::mt_home; break;
	case VK_END:   mt = control_pressed? lyt::mt_tail : lyt::mt_end; break;

	case VK_RETURN: OnPressEnter(); break;
	case VK_ESCAPE: 
		BOOL tmp;
		OnCancel(0, 0, NULL, tmp);
		break;
	}
	if (mt != lyt::mt_none)
	{
		m_lyt->move_prog_pos(&pp, mt);
		CClientDC dc(m_hWnd);
		m_lyt->draw_prog(dc, m_curr_prog_pos, lyt::normal);
		m_lyt->draw_prog(dc, pp, lyt::active);
		m_curr_prog_pos = pp;
	}

	return TRUE;
}

LRESULT CRunDlg::OnTTGetDisp(CToolTipCtrl& tip_ctrl, LPNMHDR pnmh, BOOL &)
{
	static wchar_t buf[512];
	LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)pnmh;
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	prog_pos pp = m_lyt->get_prog_from_point(pt);
	if (pp != pp_null)
	{
		const prog * p = g_c->get_prog(pp.g, pp.p);
		std::wstring abspath = hlp::abs_path(p->path.c_str());
		hlp::path_elem pe = hlp::split_path(abspath.c_str());
		std::wstring name = abspath.substr(pe.name.s, pe.name.n)
		                  + L" -- "
						  + g_c->get_group_info(pp.g)->name;
		if (g_c->gm.use_simple_tip)
		{
			tip_ctrl.SetTitle(TTI_NONE, hc::empty_str);
		}
		else
		{
			tip_ctrl.SetTitle(TTI_INFO, name.c_str());
		}

		buf[0] = L'\0';
		size_t bl = sizeof(buf)/sizeof(buf[0]);
		size_t pos = 0;
		pos = hlp::strcat_ex(buf, bl, pos, p->path.c_str());
		if (!p->param.empty())
		{
			pos = hlp::strcat_ex(buf, bl, pos, L" ");
			pos = hlp::strcat_ex(buf, bl, pos, p->param.c_str());
		}
		if (!p->comment.empty())
		{
			pos = hlp::strcat_ex(buf, bl, pos, L"\r\n");
			pos = hlp::strcat_ex(buf, bl, pos, p->comment.c_str());
		}
		
		pDispInfo->lpszText = buf;
	}
	return TRUE;
}

LRESULT CRunDlg::OnEnChangeProgEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tip.OnInputChange();
	return 0;
}

void CRunDlg::OnClickProg(const prog *p)
{
	bool ret;
	if (hlp::is_key_down(VK_CONTROL))
	{
		prog new_p = *p;
		CInputDlg dlg(L"输入参数", p->path.c_str(), p->param.c_str());
		if (dlg.DoModal() == IDCANCEL) return;
		new_p.param = dlg.get_value();
		ret = DoExecute(&new_p);
	}
	else
	{
		ret = DoExecute(p);
	}

	if (ret)
	{
		EndRequest();
	}
}

void CRunDlg::OnPressEnter()
{
	// 非hoverstate按回车键不执行程序
	if (m_os != &m_os_hover) return;

	prog tmp_p;
	command *p = NULL;
	bool forced = false;

	// 强制执行输入的命令
	if (m_edit.IsWindowVisible() && m_edit.GetWindowTextLengthW() > 0 &&
		(hlp::is_key_down(VK_CONTROL) || !m_tip.IsWindowVisible()))
	{
		forced = true;
		wchar_t buf[256];
		m_edit.GetWindowTextW(buf, elem_of(buf));
		bool in_quote = false;
		int i;
		for (i = 0; buf[i]; i++)
		{
			if (buf[i] == L'\"') in_quote = !in_quote;
			if (buf[i] == L' ' && !in_quote) break;
		}
		tmp_p.path.assign(buf, i);
		if (buf[i])
		{
			tmp_p.param = buf + i + 1;
		}
		p = &tmp_p;
	}

	// 提示框
	if (p == NULL)
	{
		if (m_tip.IsWindowVisible())
			p = m_tip.GetSelectCmd();
	}

	// 键盘导航
	if (p == NULL) p = g_c->get_prog(m_curr_prog_pos.g, m_curr_prog_pos.p);

	if (p == NULL)
	{
		enable_type_command(!m_edit.IsWindowVisible());
	}
	else
	{
		m_last_cmd_forced = forced;
	}
	
	if (DoExecute(p))
	{
		EndRequest();
	}
}

LRESULT CRunDlg::OnContextMenu(UINT , WPARAM , LPARAM lp, BOOL &)
{
	cfg::config *c = cfg::config::instance();
	CMenu menu;
	menu.LoadMenuW(IDR_MENU_POPUP);
	CMenu mu = (HMENU)menu.GetSubMenu(0);

	POINT pt = {-1, -1};
	if (lp == 0xFFFFFFFF) // 菜单不是右键激活的而是按键激活的
	{
		hlp::track_mouse_event(TME_CANCEL|TME_LEAVE, m_hWnd);
		if (m_curr_prog_pos != pp_null)
		{
			lyt::icon_layout *il = dynamic_cast<lyt::icon_layout *>(m_lyt);
			if (il != NULL)
			{
				RECT rc = il->get_prog_rect(m_curr_prog_pos);
				m_menu_point.x = (rc.left + rc.right) / 2;
				m_menu_point.y = (rc.top + rc.bottom) / 2;
				pt = m_menu_point;
				ClientToScreen(&pt);
			}
		}
	}
	if (pt.x == -1)
	{
		::GetCursorPos(&pt);
		m_menu_point = pt;
		ScreenToClient(&m_menu_point);
	}

	if (c->gm.layout != cfg::ul_group)
	{
		mu.DeleteMenu(ID_INSERT_GROUP, MF_BYCOMMAND);
		mu.DeleteMenu(ID_EDIT_GROUP, MF_BYCOMMAND);
		mu.DeleteMenu(ID_DEL_GROUP, MF_BYCOMMAND);
	}
	if (c->gm.layout == cfg::ul_simple)
	{
		mu.DeleteMenu(ID_INSERT_PROG, MF_BYCOMMAND);
		mu.DeleteMenu(ID_EDIT_PROG, MF_BYCOMMAND);
		mu.DeleteMenu(ID_DEL_PROG, MF_BYCOMMAND);
	}

	if (m_lyt->get_insert_pos(m_menu_point) == pp_null)
	{
		mu.EnableMenuItem(ID_INSERT_PROG, MF_BYCOMMAND | MF_GRAYED);
//		mu.EnableMenuItem(ID_INSERT_GROUP, MF_BYCOMMAND | MF_GRAYED);
		mu.EnableMenuItem(ID_EDIT_GROUP, MF_BYCOMMAND | MF_GRAYED);
		mu.EnableMenuItem(ID_DEL_GROUP, MF_BYCOMMAND | MF_GRAYED);
	}
	if (m_lyt->get_prog_from_point(m_menu_point) == pp_null)
	{
		mu.EnableMenuItem(ID_EDIT_PROG, MF_BYCOMMAND | MF_GRAYED);
		mu.EnableMenuItem(ID_DEL_PROG, MF_BYCOMMAND | MF_GRAYED);
	}
	// 显示样式前面的对勾
	UINT lyt_id;
	switch (c->gm.layout)
	{
	case cfg::ul_simple: lyt_id = ID_CHLYT_SIMPLE; break;
	case cfg::ul_plain:  lyt_id = ID_CHLYT_PLAIN; break;
	default:             lyt_id = ID_CHLYT_GROUP; break;
	}
	mu.CheckMenuItem(lyt_id, MF_BYCOMMAND|MF_CHECKED);
	UINT style_id;
	switch (c->gm.style)
	{
	case cfg::ws_edge:  style_id = ID_CHSTYLE_EDGE; break;
	default:            style_id = ID_CHSTYLE_CENTER; break;
	}
	mu.CheckMenuItem(style_id, MF_BYCOMMAND|MF_CHECKED);

	// 删除多余的分隔线
	for (int i = mu.GetMenuItemCount() - 1; i > 0; i--)
	{
		if (mu.GetMenuStringLen(i,   MF_BYPOSITION) == 0 &&
			mu.GetMenuStringLen(i-1, MF_BYPOSITION) == 0)
		{
			mu.DeleteMenu(i, MF_BYPOSITION);
		}
	}

	KillTimer(IDT_AUTO_FOLD); // 菜单消失前不能让窗口自动隐藏
	mu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	change_op_state(os_hover);
	return TRUE;
}

LRESULT CRunDlg::OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSimpleDialog<IDD_ABOUT> ab;
	ab.DoModal(m_hWnd);
	return 0;
}

LRESULT CRunDlg::OnChangeProfile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	struct local
	{
		static void change_profile()
		{
			std::wstring path = gd::hr_ini;
			CFileDialog dlg(TRUE, NULL, path.c_str(), OFN_FILEMUSTEXIST);
			if (dlg.DoModal() == IDOK)
			{
				g_c->set_profile(dlg.m_szFileName);
			}
		}
	};
	reload_profile(local::change_profile);
	return 0;
}

LRESULT CRunDlg::OnEditProfile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	struct local
	{
		static void edit_profile()
		{
			std::wstring path = g_c->get_profile().c_str();
			SHELLEXECUTEINFO sei = {
				sizeof(SHELLEXECUTEINFO), SEE_MASK_NOCLOSEPROCESS, NULL, 
				L"open", path.c_str(), 
				NULL, NULL, SW_SHOW
			};
			if (::ShellExecuteEx(&sei))
			{
				::WaitForSingleObject(sei.hProcess, INFINITE);
				::CloseHandle(sei.hProcess);
			}
			else
			{
				hlp::show_err(os_err(win).what(), NULL);
			}
		}
	};
	reload_profile(local::edit_profile);
	return 0;
}

LRESULT CRunDlg::OnEditProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	prog_pos pp = m_lyt->get_prog_from_point(m_menu_point);
	prog * pi = g_c->get_prog(pp.g, pp.p);
	CProgDlg dlg(*pi);
	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		refresh_layout();
		show_window(true);
	}

	return 0;
}

LRESULT CRunDlg::OnInsertProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	prog_pos pp = m_lyt->get_insert_pos(m_menu_point);
	if (pp == lyt::pp_null) return -1;

	prog new_pi;
	new_pi.icon.index = 0;
	CProgDlg dlg(new_pi);
	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		g_c->group_mgr.insert_cmd(new prog(new_pi), pp.g, pp.p);
		refresh_layout();
		show_window(true);
	}

	return 0;
}


LRESULT CRunDlg::OnDelProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	prog_pos pp = m_lyt->get_prog_from_point(m_menu_point);
	g_c->group_mgr.delete_cmd(pp.g, pp.p);
	refresh_layout();
	show_window(true);
	
	return 0;
}

LRESULT CRunDlg::OnImportProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CImportProgDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CRunDlg::OnChlytGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	g_c->gm.layout = cfg::ul_group;
	refresh_layout();
	show_window(true);
	return 0;
}

LRESULT CRunDlg::OnChlytPlain(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	g_c->gm.layout = cfg::ul_plain;
	refresh_layout();
	show_window(true);
	return 0;
}

LRESULT CRunDlg::OnChlytSimple(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	g_c->gm.layout = cfg::ul_simple;
	refresh_layout();
	show_window(true);
	return 0;
}

LRESULT CRunDlg::OnChstyleCenter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	g_c->gm.style = cfg::ws_center;
	show_window(true);
	return 0;
}

LRESULT CRunDlg::OnChstyleEdge(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	g_c->gm.style = cfg::ws_edge;
	show_window(true);
	return 0;
}

LRESULT CRunDlg::OnExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ui_exit();
	return 0;
}

LRESULT CRunDlg::OnInsertGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	prog_pos pp = m_lyt->get_insert_pos(m_menu_point);
	group_info new_gi;
	new_gi.color = RGB(128, 128, 128);
	CGroupDlg dlg(new_gi, true);
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_insert_before)
		{
			if (pp == lyt::pp_null) pp.g = 0;
		}
		else
		{
			if (pp == lyt::pp_null) pp.g = g_c->group_mgr.cs_count();
			else pp.g++;
		}
		
		g_c->group_mgr.insert_cs(new group_info(new_gi), pp.g);
		refresh_layout();
		show_window(true);
	}
	return 0;
}

LRESULT CRunDlg::OnEditGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	prog_pos pp = m_lyt->get_insert_pos(m_menu_point);
	group_info *gi = g_c->get_group_info(pp.g);
	CGroupDlg dlg(*gi, false);
	if (dlg.DoModal() == IDOK)
	{
		RedrawWindow();
	}
	return 0;
}

LRESULT CRunDlg::OnDelGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	const wchar_t * info = L"确实要删除组及组内的所有程序吗？";
	if (MessageBox(info, L"确认", MB_OKCANCEL) == IDOK)
	{
		prog_pos pp = m_lyt->get_insert_pos(m_menu_point);
		g_c->group_mgr.delete_cs(pp.g);
		refresh_layout();
		show_window(true);
	}
	return 0;
}

HRESULT CRunDlg::OnDragEnter(IDataObject * pDataObject, DWORD , POINTL ptl, DWORD *pdwEffect)
{
	change_op_state(os_drop_target);
	POINT pt = {ptl.x, ptl.y};
	ScreenToClient(&pt);
	*pdwEffect = m_os_drop_target.drag_enter(pDataObject, pt);
	return S_OK;
}
HRESULT CRunDlg::OnDragOver(DWORD , POINTL ptl, DWORD * pdwEffect)
{
	POINT pt = {ptl.x, ptl.y};
	ScreenToClient(&pt);
	*pdwEffect = m_os_drop_target.drag_over(pt);
	return S_OK;
}
HRESULT CRunDlg::OnDragLeave()
{
	m_os_drop_target.drag_leave();
	return S_OK;
}
HRESULT CRunDlg::OnDrop(IDataObject * pDataObject, DWORD , POINTL ptl, DWORD * pdwEffect)
{
	POINT pt = {ptl.x, ptl.y};
	ScreenToClient(&pt);
	*pdwEffect = m_os_drop_target.drop(pDataObject, pt);
	change_op_state(os_hover);
	return S_OK;
}

LRESULT CRunDlg::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	return 0;
}