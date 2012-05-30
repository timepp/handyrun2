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
	ui_layout layout;                 // ��ʽ: ����/ƽ̹/���
	window_style style;               // ���: ����/��Ե

	bool use_tip;                     // �Ƿ���ʾ������ʾ
	bool use_simple_tip;              // �Ƿ�ʹ�õ�����ʾ
	bool hide_on_lose_focus;          // ʧȥ����ʱ�Ƿ�����
	bool cache_icon;                  // �Ƿ񻺴�ͼ��

	int border;                       // �߽��С(��������)
	int pad_h;                        // ͼ��ˮƽ���
	int pad_v;                        // ͼ�괹ֱ���
	int icon_size;                    // ͼ���С(��=��)
	int icon_per_line;                // ƽ̹ģʽʱÿ�е�ͼ������
	int edit_height;                  // ���������ĸ߶�
	int min_window_width;             // ��С�Ĵ��ڿ��
	edge_class edge;                  // [��Եģʽ]λ��: ��/��/��/��
	int edge_pos;                     // [��Եģʽ]λ��: ����
	int edge_window_len;              // [��Եģʽ]���ڳ���
	int edge_window_thickness;        // [��Եģʽ]���ں��

	gui_metrics();                    // ����ȱʡֵ
};

struct general_opt
{
	std::wstring active_key;          // �����ȼ�
	std::wstring executable_ext;      // "����"����չ��
	bool single_instance;             // �Ƿ�ʵ������
	bool keep_dos_cmd_window;         // �Ƿ�����DOS�����������

	general_opt();                    // ����ȱʡֵ
};

// singleton
class config
{
public:
	~config();
	static config * instance();

	cs_mgr group_mgr;                    // ������������
	cs_mgr index_mgr;                    // ����������
	gui_metrics gm;                      // ����Ԫ������
	general_opt go;                      // һ��ѡ��
	background  bk;                      // ����

	void set_profile(const wchar_t * fn);
	std::wstring get_profile() const;
	bool save();
	bool load();

	// ��ݷ�ʽ
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
	std::vector<env_var> m_evs;          // ����Ļ�������
};

}
