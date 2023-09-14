#pragma once

#include "prog.h"
#include "defs.h"
#include "cs_mgr.h"
#include <list>

namespace cfg
{

	enum ui_layout {ul_group, ul_plain, ul_simple, ul_none};
	enum window_style {ws_center, ws_edge, ws_none};
	enum edge_class {ec_t, ec_l, ec_r, ec_b, ec_none};

struct gui_metrics
{
	ui_layout layout;                 // 样式: 分组/平坦/简洁
	window_style style;               // 风格: 居中/边缘

	bool use_tip;                     // 是否显示程序提示
	bool use_simple_tip;              // 是否使用单行提示
	bool hide_on_lose_focus;          // 失去焦点时是否隐藏
	bool cache_icon;                  // 是否缓存图标

	int border;                       // 边界大小(上下左右)
	int pad_h;                        // 图标水平间距
	int pad_v;                        // 图标垂直间距
	int icon_size;                    // 图标大小(宽=高)
	int icon_per_line;                // 平坦模式时每行的图标数量
	int edit_height;                  // 命令输入框的高度
	int min_window_width;             // 最小的窗口宽度
	edge_class edge;                  // [边缘模式]位置: 左/上/右/下
	int edge_pos;                     // [边缘模式]位置: 距离
	int edge_window_len;              // [边缘模式]窗口长度
	int edge_window_thickness;        // [边缘模式]窗口厚度

	gui_metrics();                    // 设置缺省值
};

struct general_opt
{
	std::wstring active_key;          // 激活热键
	std::wstring executable_ext;      // "程序"的扩展名
	bool single_instance;             // 是否单实例运行
	bool keep_dos_cmd_window;         // 是否运行DOS命令后保留窗口

	general_opt();                    // 设置缺省值
};

// singleton
class config
{
public:
	~config();
	static config * instance();

	cs_mgr group_mgr;                    // 程序分组管理器
	cs_mgr index_mgr;                    // 索引管理器
	gui_metrics gm;                      // 界面元素配置
	general_opt go;                      // 一般选项
	background  bk;                      // 背景

	void set_profile(const wchar_t * fn);
	std::wstring get_profile() const;
	bool save();
	bool load();

	// 快捷方式
	prog * get_prog(int g, int p);
	group_info * get_group_info(int g);
	command * get_index_cmd(int i, int c);
	index_info * get_index_info(int i);

private:
	config();
	config(const config&);
	config& operator = (const config&);

	void cleanup();
	void add_env_var();
	void del_env_var();
	void fill_index_prog();

	std::wstring m_profile;
	std::vector<env_var> m_evs;          // 额外的环境变量
};

}
