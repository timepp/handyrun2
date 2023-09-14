#include "stdafx.h"

#include "config.h"
#include "helper.h"
#include "error.h"
#include "defs.h"
#include "path_lister.h"
#include "global_data.h"
#include "log.h"
#include <errno.h>
#include <stdio.h>

using namespace cfg;
namespace
{

struct si_pair
{
	const wchar_t * str;
	int int_val;
};

// 配置文件中键值的绑定信息
struct val_desc
{
	const wchar_t * key;          // 键
	void * val_ptr;               // 指向值的指针
	value_type vt;                // 值类型
	// 缺省值, 当选项的值为缺省值时, 不向配置文件中写入
	const wchar_t * default_val;
	// 字符串-整型映射表(仅vt == vt_int时有效)
	// 配置文件中是字符串形式,程序内部是整数形式,使用si_map做转换
	const si_pair * si_map;
};
typedef std::list<val_desc> binding_t;

static const si_pair ul_map[] = 
{
	L"group",  ul_group,
	L"plain",  ul_plain,
	L"simple", ul_simple,
	NULL, 0
};
static const si_pair ws_map[] = 
{
	L"center", ws_center,
	L"edge",   ws_edge,
	NULL, 0
};
static const si_pair ec_map[] = 
{
	L"top", ec_t, L"left", ec_l, L"right", ec_r, L"bottom", ec_b, NULL, 0
};
static const si_pair st_map[] =
{
	L"",     SW_SHOW,
	L"min",  SW_SHOWMINNOACTIVE,
	L"max",  SW_SHOWMAXIMIZED,
	L"back", SW_SHOWNOACTIVATE,
	L"hide", SW_HIDE,
	NULL, 0
};
static const si_pair bkmode_map[] =
{
	L"color", background::bm_color,
	L"img",   background::bm_img,
	NULL,     0
};

// 在映射表中查找
static int lookup(const si_pair* si_map, const wchar_t * str)
{
	for (int i = 0; si_map[i].str != NULL; i++)
	{
		if (_wcsicmp(str, si_map[i].str) == 0) return si_map[i].int_val;
	}
	return si_map[0].int_val;
}
static const wchar_t * lookup(const si_pair* si_map, int v)
{
	for (int i = 0; si_map[i].str != NULL; i++)
	{
		if (v == si_map[i].int_val) return si_map[i].str;
	}
	return si_map[0].str;
}

// 往绑定表里添加一条记录
// bind : 不检查重复
// bind_uniq : 检查重复
static void bind(binding_t * b, const wchar_t *k, void *v, value_type t, const wchar_t * dv = NULL, const si_pair * sip = NULL)
{
	val_desc vd = {k, v, t, dv, sip};
	b->push_back(vd);
}
static void bind_uniq(binding_t * b, const wchar_t *k, void *v, value_type t, const wchar_t * dv = NULL, const si_pair * sip = NULL)
{
	val_desc vd = {k, v, t, dv, sip};
	for (binding_t::iterator it = b->begin(); it != b->end(); ++it)
	{
		if (wcscmp(it->key, k) == 0)
		{
			*it = vd;
			return;
		}
	}
	b->push_back(vd);
}

// 设置各种绑定信息

static void bind_general_option(binding_t * b, general_opt& go)
{
	bind(b, L"single_instance", &go.single_instance, vt_bool);
	bind(b, L"activate_key", &go.active_key, vt_wstring);
	bind(b, L"keep_dos_cmd_window", &go.keep_dos_cmd_window, vt_bool);
	bind(b, L"executable_ext", &go.executable_ext, vt_wstring);
}

static void bind_background_option(binding_t * b, background& bk)
{
	bind(b, L"mode", &bk.mode, vt_int, NULL, bkmode_map);
	bind(b, L"color_list", &bk.color_list, vt_color_list);
	bind(b, L"fill_direction", &bk.fill_direction, vt_direction);
	bind(b, L"img", &bk.back_img, vt_wstring);
	bind(b, L"mask", &bk.mask_img, vt_wstring);
}

static void bind_ui_metrics(binding_t * b, gui_metrics& gm)
{
	bind(b, L"layout", &gm.layout, vt_int, NULL, ul_map);
	bind(b, L"style", &gm.style, vt_int, NULL, ws_map);

	bind(b, L"use_tip", &gm.use_tip, vt_bool);
	bind(b, L"use_simple_tip", &gm.use_simple_tip, vt_bool);
	bind(b, L"hide_on_lose_focus", &gm.hide_on_lose_focus, vt_bool);
	bind(b, L"cache_icon", &gm.cache_icon, vt_bool);

	bind(b, L"border", &gm.border, vt_int);
	bind(b, L"icon_size", &gm.icon_size, vt_int);
	bind(b, L"pad_h", &gm.pad_h, vt_int);
	bind(b, L"pad_v", &gm.pad_v, vt_int);
	bind(b, L"icon_per_line", &gm.icon_per_line, vt_int);
	bind(b, L"edit_height", &gm.edit_height, vt_int);
	bind(b, L"min_window_width", &gm.min_window_width, vt_int);

	bind(b, L"edge", &gm.edge, vt_int, NULL, ec_map);
	bind(b, L"edge_pos", &gm.edge_pos, vt_int);
	bind(b, L"edge_window_len", &gm.edge_window_len, vt_int);
	bind(b, L"edge_window_thickness", &gm.edge_window_thickness, vt_int);
}

static void bind_prog(binding_t * b, prog * p)
{
	bind_uniq(b, L"p", &p->path, vt_wstring);
	bind_uniq(b, L"c", &p->comment, vt_wstring, hc::empty_str);
	bind_uniq(b, L"param", &p->param, vt_wstring, hc::empty_str);
	bind_uniq(b, L"dp", &p->drop_param, vt_wstring, hc::empty_str);
	bind_uniq(b, L"wd", &p->work_dir, vt_wstring, hc::empty_str);
	bind_uniq(b, L"icon", &p->icon, vt_icon, hc::empty_str);
	bind_uniq(b, L"show_type", &p->show_cmd, vt_int, hc::empty_str, st_map);
}

static void bind_group(binding_t * b, group_info * gi)
{
	bind(b, L"color", &gi->color, vt_color);
}

static void bind_index(binding_t * b, index_info * ii)
{
	bind_uniq(b, L"path", &ii->path, vt_wstring);
	bind_uniq(b, L"depth", &ii->depth, vt_int, L"1");
	bind_uniq(b, L"icon", &ii->icon, vt_icon, hc::empty_str);
}

static std::wstring val_to_str(const void * val, value_type vt)
{
	wchar_t buf[64];
	if (vt == vt_int)
	{
		_itow_s(*static_cast<const int *>(val), buf, elem_of(buf), 10);
		return buf;
	}
	else if (vt == vt_bool)
	{
		return *static_cast<const bool *>(val) ? L"1" : L"0";
	}
	else if (vt == vt_wstring)
	{
		return *static_cast<const std::wstring *>(val);
	}
	else if (vt == vt_double)
	{
		swprintf_s(buf, elem_of(buf), L"%lf", *static_cast<const double *>(val));
		return buf;
	}
	else if (vt == vt_color)
	{
		COLORREF cr = *static_cast<const COLORREF *>(val);
		if ((cr & 0xFF000000) == 0)
		{
			_snwprintf_s(buf, elem_of(buf), _TRUNCATE, L"#%02X%02X%02X", 
				GetRValue(cr), GetGValue(cr), GetBValue(cr));
			return buf;
		}
	}
	else if (vt == vt_icon)
	{
		const icon_desc * d = static_cast<const icon_desc *>(val);
		buf[0] = L'\0';
		if (d->index > 0)
		{
			_snwprintf_s(buf, elem_of(buf), _TRUNCATE, L"|%d", d->index);
		}
		return d->file + buf;
	}
	else if (vt == vt_direction)
	{
		int dir = *static_cast<const int *>(val);
		std::wstring ret;
		switch (dir)
		{
		case 0:                     ret = L"-";  break;
		case 270:                   ret = L"|";  break;
		case background::sd_diag:   ret = L"\\"; break;
		case background::sd_rdiag:  ret = L"/";  break;
		default:
			ret = val_to_str(val, vt_int);
			break;
		}
		return ret;
	}
	else if (vt == vt_color_list)
	{
		std::wstring ret;
		const color_list_t * cl = static_cast<const color_list_t *>(val);
		for (color_list_t::const_iterator it = cl->begin(); it != cl->end(); ++it)
		{
			COLORREF cr = *it;
			if (!ret.empty()) ret += L" -> ";
			ret += val_to_str(&cr, vt_color);
		}
		return ret;
	}

	return L"";
}

static bool str_to_val(const wchar_t *str, void *val, value_type vt)
{
	if (vt == vt_int)
	{
		*static_cast<int *>(val) = _wtoi(str);
	}
	else if (vt == vt_bool)
	{
		bool * b = static_cast<bool *>(val);
		*b = false;
		if (_wcsicmp(str, L"true") == 0 || _wtoi(str) == 1)
		{
			*b = true;
		}
	}
	else if (vt == vt_wstring)
	{
		*static_cast<std::wstring *>(val) = str;
	}
	else if (vt == vt_double)
	{
		*static_cast<double *>(val) = _wtof(str);
	}
	else if (vt == vt_color)
	{
		const wchar_t * s = str;
		COLORREF * cr = static_cast<COLORREF *>(val);
		if (*s == L'#' && wcslen(s) >= 6)
		{
			s++;
			*cr = RGB(hlp::hex_val(s, 2), hlp::hex_val(s+2, 2), hlp::hex_val(s+4, 2));
			return true;
		}
		else
		{
			*cr = 0xFF000000;
			return false;
		}
	}
	else if (vt == vt_icon)
	{
		icon_desc * d = static_cast<icon_desc *>(val);
		const wchar_t *p = wcsrchr(str, L'|');
		if (p != NULL)
		{
			d->file.assign(str, p - str);
			d->index = _wtoi(p + 1);
		}
		else
		{
			d->file = str;
			d->index = 0;
		}
	}
	else if (vt == vt_direction)
	{
		int * dir = static_cast<int *>(val);
		switch (str[0])
		{
		case '-':  if (!str[1]) {*dir = 0;  return true;}
				   break;
		case '|':  *dir = 270; return true;
		case '\\': *dir = background::sd_diag;   return true;
		case '/':  *dir = background::sd_rdiag;  return true;
		}
		return str_to_val(str, val, vt_int);
	}
	else if (vt == vt_color_list)
	{
		color_list_t * cl = static_cast<color_list_t *>(val);
		cl->clear();
		COLORREF cr;
		while (str_to_val(str, &cr, vt_color))
		{
			cl->push_back(cr);
			str += 7;
			while (*str && *str != '#') str++;
		}
		return true;
	}
	else
	{
		return false;
	}
	return true;
}

static void save_sec(FILE *fp, const wchar_t * sec)
{
	fwprintf(fp, L"[%s]\n", sec);
}

static void save_key(FILE *fp, const wchar_t *key, const wchar_t *val, int key_field_width = 0)
{
	wchar_t fmt[32];
	_snwprintf_s(fmt, elem_of(fmt), _TRUNCATE, L"%%-%ds = %%s\n", key_field_width);
	fwprintf(fp, fmt, key, val);
}
static void save_key(FILE *fp, const binding_t &b, int key_field_width = 0)
{
	std::wstring s;
	for (binding_t::const_iterator ci = b.begin(); ci != b.end(); ++ci)
	{
		if (ci->si_map && ci->vt == vt_int)
		{
			s = lookup(ci->si_map, *static_cast<const int *>(ci->val_ptr));
		}
		else
		{
			s = val_to_str(ci->val_ptr, ci->vt);
		}
		if (ci->default_val == NULL || s != ci->default_val)
		{
			save_key(fp, ci->key, s.c_str(), key_field_width);
		}
	}
}

static void load_key(const wchar_t *key, const wchar_t *val, const binding_t &b)
{
	for (binding_t::const_iterator ci = b.begin(); ci != b.end(); ++ci)
	{
		if (_wcsicmp(ci->key, key) == 0)
		{
			if (ci->si_map && ci->vt == vt_int)
			{
				*static_cast<int *>(ci->val_ptr) = lookup(ci->si_map, val);
			}
			else
			{
				str_to_val(val, ci->val_ptr, ci->vt);
			}
		}
	}
}

// 打开配置文件
// forload: true : 读取
//          false: 写入
static FILE * open_profile(const std::wstring& fn, bool for_load)
{
	FILE *fp;
	const wchar_t * mode = for_load? L"r,ccs=UNICODE" : L"w, ccs=UTF-8";
	const wchar_t * op = for_load? L"读取配置文件" : L"保存配置文件";
	if (_wfopen_s(&fp, fn.c_str(), mode) != 0)
	{
		throw os_err(dos, op, L"  文件名:%s", fn.c_str());
	}
	return fp;
}

static void close_profile(FILE *fp)
{
	fclose(fp);
}

} // anonymous namespace

///////////////////////////////////////////////////////////

gui_metrics::gui_metrics() :
	layout(ul_group),            use_tip(true), 
	style(ws_center),            use_simple_tip(false),
	edge(ec_t),                  hide_on_lose_focus(true),
	                             cache_icon(false),

	border(7),                   icon_size(32),
	pad_h(5),                    icon_per_line(10),
	pad_v(15),                   edge_pos(200),
	edit_height(25),             edge_window_len(100),
	min_window_width(400),       edge_window_thickness(5)
{
}

general_opt::general_opt() 
: active_key(L"\"LCONTROL")
, executable_ext(L"exe lnk")
, single_instance(false)
, keep_dos_cmd_window(true)
{
}

config::config()
{
	m_profile = gd::hr_path + hc::fn_ini;
	load();
}

config::~config()
{
	save();
	cleanup();
}

config * config::instance()
{
	static config c;
	return &c;
}

void config::cleanup()
{
	del_env_var();
	m_evs.clear();
	group_mgr.clear();
	index_mgr.clear();
}

void config::set_profile(const wchar_t * fn)
{
	m_profile = fn;
}
std::wstring config::get_profile() const
{
	return m_profile;
}

prog * config::get_prog(int g, int p)
{
	return dynamic_cast<prog *>(group_mgr.get_cmd(g, p));
}
group_info * config::get_group_info(int g)
{
	return dynamic_cast<group_info *>(group_mgr.get_cs_info(g));
}
command * config::get_index_cmd(int i, int c)
{
	return index_mgr.get_cmd(i, c);
}
index_info * config::get_index_info(int i)
{
	return dynamic_cast<index_info *>(index_mgr.get_cs_info(i));
}

bool config::load()
{
	log_msg(L"loading config");
	cleanup();

	FILE * fp;
	try
	{
		fp = open_profile(m_profile, true);
	}
	catch (os_err &he)
	{
		if (he.m_err_code == ENOENT)
		{
			// 可能是第一次运行HandyRun
			// TODO: 一些友好的提示
		}
		else
		{
			hlp::show_err(he.what(), L"将使用缺省参数运行程序");
		}
		return false;
	}

	binding_t b;
	wchar_t line[1024];
	wchar_t sec[64] = L"";
	while (fgetws(line, elem_of(line), fp))
	{
		if (line[0] == L';' || line[0] == L'#') continue;

		wchar_t *p, *val, *key;
		if (line[0] == L'[')
		{
			p = wcschr(line, L']');
			if (!p) continue;

			wcsncpy_s(sec, elem_of(sec), line + 1, p - line - 1);
			// 遇到新的段
			b.clear();
			if (_wcsicmp(sec, L"general") == 0)
			{
				bind_general_option(&b, go);
			}
			else if (_wcsicmp(sec, L"ui") == 0)
			{
				bind_ui_metrics(&b, gm);
			}
			else if (_wcsicmp(sec, L"background") == 0)
			{
				bind_background_option(&b, bk);
			}
			else if (_wcsnicmp(sec, L"group:", 6) == 0)
			{
				group_info * gi = new group_info(sec + 6);
				group_mgr.insert_cs(gi);
				bind_group(&b, gi);
			}
		}
		else
		{
			p = wcschr(line, L'=');
			if (!p) continue;
			// 获取key和val的起始地址
			for (key = line; hlp::is_space(*key); key++);
			for (val = p + 1; hlp::is_space(*val); val++);
			// 移除多余的空白
			for (p--; p > line && hlp::is_space(*p); p--); 
			*(p+1) = L'\0';
			for (p = val; *p && *p != L'\n'; p++); 
			*p = L'\0';
			// 遇到新的键值([sec] key = val)
			if (_wcsicmp(sec, L"var") == 0)
			{
				env_var ev = {key, val};
				m_evs.push_back(ev);
			}
			else 
			{
				if (_wcsnicmp(sec, L"group:", 6) == 0 && _wcsicmp(key, L"p") == 0)
				{
					prog * pg = new prog;
					group_mgr.insert_cmd(pg, -1);
					bind_prog(&b, pg);
				}
				if (_wcsicmp(sec, L"index_path") == 0 && _wcsicmp(key, L"path") == 0)
				{
					index_info *ii = new index_info();
					index_mgr.insert_cs(ii);
					bind_index(&b, ii);
				}
				// 按当前的key和val,还有绑定表存储键值
				load_key(key, val, b);
			}
		}
	}

	close_profile(fp);
	add_env_var();
	fill_index_prog();

	return true;
}

bool config::save()
{
	FILE * fp;
	try
	{
		fp = open_profile(m_profile, false);
	}
	catch (...)
	{
		return false;
	}

	binding_t b;

	// general options
	save_sec(fp, L"general");	
	bind_general_option(&b, go);
	save_key(fp, b);
	fwprintf(fp, hc::newline_str);
	// gui metrics
	b.clear();
	save_sec(fp, L"ui");
	bind_ui_metrics(&b, gm);
	save_key(fp, b);
	fwprintf(fp, hc::newline_str);
	// background
	b.clear();
	save_sec(fp, L"background");
	bind_background_option(&b, bk);
	save_key(fp, b);
	fwprintf(fp, hc::newline_str);

	int fw = 12;  // field width
	// env var
	const wchar_t * sec = L"var";
	save_sec(fp, sec);
	for (size_t i = 0; i < m_evs.size(); i++)
	{
		env_var &ev = m_evs[i];
		save_key(fp, ev.key.c_str(), ev.val.c_str(), fw);
	}
	fwprintf(fp, hc::newline_str);

	b.clear();
	save_sec(fp, L"index_path");
	for (int i = 0; i < index_mgr.cs_count(); i++)
	{
		bind_index(&b, get_index_info(i));
		save_key(fp, b, fw);
	}

	for (int i = 0; i < group_mgr.cs_count(); i++)
	{
		fwprintf(fp, hc::newline_str);
		group_info * gi = get_group_info(i);
		save_sec(fp, tp::cz(L"group:%s", gi->name.c_str()));
		b.clear();
		bind_group(&b, gi);
		save_key(fp, b, fw);
		b.clear();
		for (int j = 0; j < group_mgr.cmd_count(i); j++)
		{
			bind_prog(&b, get_prog(i,j));
			save_key(fp, b, fw);
		}
	}

	close_profile(fp);
	return true;
}

void config::add_env_var()
{
	for (size_t i = 0; i < m_evs.size(); i++)
	{
		::SetEnvironmentVariable(m_evs[i].key.c_str(), m_evs[i].val.c_str());
	}
}

void config::del_env_var()
{
	for (size_t i = 0; i < m_evs.size(); i++)
	{
		::SetEnvironmentVariable(m_evs[i].key.c_str(), NULL);
	}
}

void config::fill_index_prog()
{
	try
	{
		std::wstring p, n, ext, base_path;
		for (int i = 0; i < index_mgr.cs_count(); i++)
		{
			index_info * ii = get_index_info(i);
			base_path = hlp::abs_path(ii->path.c_str());
			path_lister pl(base_path.c_str(), ii->depth);
			while (pl.get_next_file(p, n))
			{
				hlp::path_elem pe = hlp::split_path(n.c_str());
				ext.assign(n, pe.ext.s, pe.ext.n);
				if (hlp::match_str_list(go.executable_ext.c_str(), ext.c_str() + 1) >= 0)
				{
					command * c = new command;
					c->path = base_path + L"\\" + p + L"\\" + n;
					index_mgr.insert_cmd(c, i);
				}
				//ATLTRACE2(atlTraceGeneral, 0, "%s\n", pi.absolute_path.c_str());
			}
		}
	}
	catch (handyrun_err& he)
	{
		hlp::show_err(he.what(), L"对该目录的命令提示可能不完整。");
	}
}
