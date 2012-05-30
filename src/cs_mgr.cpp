#include "stdafx.h"
#include "cs_mgr.h"

cs_mgr::cs_mgr()
{
}

cs_mgr::~cs_mgr()
{
	clear();
}

void cs_mgr::clear()
{
	while (delete_cs(0)) {}
}

// ������Ĳ����е�������������λ��,�����������:
// 1. *p��[0,  N)��Χ֮��, *p����
// 2. *p��[-N, 0)��Χ֮��, ����*p(��-1���N-1, -2���N-2, ...)
// 3. �������, ����ʧ��
static bool adjust_pos(int *p, int N)
{
	if (*p < 0 && *p >= -N) *p += N;
	if (*p < 0 || *p >=  N) return false;
	return true;
}

// ��ָ��λ�ò�������, ���븺��ʱ��β������
bool cs_mgr::insert_cs(info * cs_info, int g)
{
	if (!adjust_pos(&g, cs_count() + 1)) return false;
	command_set *cs = new command_set;
	cs->si = cs_info;
	m_css.insert(m_css.begin() + g, cs);
	return true;
}

// ɾ����, ���븺����β������
bool cs_mgr::delete_cs(int g)
{
	if (!adjust_pos(&g, cs_count())) return false;
	delete m_css[g];
	m_css.erase(m_css.begin() + g);
	return true;
}

// ��ָ��λ�ò����³���, ���븺����β������
bool cs_mgr::insert_cmd(command * c, int g, int p)
{
	if (!adjust_pos(&g, cs_count())) return false;
	if (!adjust_pos(&p, cmd_count(g) + 1)) return false;
	command_set *cs = m_css[g];
	cs->cv.insert(cs->cv.begin() + p, c);
	return true;
}

// ɾ������,���븺����Ӻ�����
bool cs_mgr::delete_cmd(int g, int p)
{
	if (!adjust_pos(&g, cs_count())) return false;
	if (!adjust_pos(&p, cmd_count(g))) return false;
	command_set * cs = m_css[g];
	delete cs->cv[p];
	cs->cv.erase(cs->cv.begin() + p);
	return true;
}

// �����������
int cs_mgr::cs_count() const
{
	return static_cast<int>(m_css.size());
}

// �������ڳ�������,���븺����Ӻ�����
int cs_mgr::cmd_count(int g) const
{
	if (!adjust_pos(&g, cs_count())) return -1;
	return static_cast<int>(m_css[g]->cv.size());
}

// �������г��������
int cs_mgr::cmd_count() const
{
	int count = 0;
	for (int i = 0; i < cs_count(); i++)
	{
		count += cmd_count(i);
	}
	return count;
}

// ���������Ƿ����, �˺���û�л��Ƶ�˵��
bool cs_mgr::cmd_exist(int g, int p) const
{
	if (g < 0 || g >= cs_count()) return false;
	if (p < 0 || p >= cmd_count(g)) return false;
	return true;
}

// ȡָ������, ���븺��ʱ��β������
command * cs_mgr::get_cmd(int g, int p) const
{
	if (!adjust_pos(&g, cs_count())) return NULL;
	if (!adjust_pos(&p, cmd_count(g))) return NULL;
	return m_css[g]->cv[p];
}

// ȡָ������Ϣ
info * cs_mgr::get_cs_info(int g) const
{
	if (!adjust_pos(&g, cs_count())) return NULL;
	return m_css[g]->si;
}
