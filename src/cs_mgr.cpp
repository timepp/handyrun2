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

// 在下面的操作中调整数组索引的位置,具体规则如下:
// 1. *p在[0,  N)范围之内, *p不变
// 2. *p在[-N, 0)范围之内, 回绕*p(即-1变成N-1, -2变成N-2, ...)
// 3. 其它情况, 函数失败
static bool adjust_pos(int *p, int N)
{
	if (*p < 0 && *p >= -N) *p += N;
	if (*p < 0 || *p >=  N) return false;
	return true;
}

// 在指定位置插入新组, 传入负数时从尾部算起
bool cs_mgr::insert_cs(info * cs_info, int g)
{
	if (!adjust_pos(&g, cs_count() + 1)) return false;
	command_set *cs = new command_set;
	cs->si = cs_info;
	m_css.insert(m_css.begin() + g, cs);
	return true;
}

// 删除组, 传入负数从尾部算起
bool cs_mgr::delete_cs(int g)
{
	if (!adjust_pos(&g, cs_count())) return false;
	delete m_css[g];
	m_css.erase(m_css.begin() + g);
	return true;
}

// 在指定位置插入新程序, 传入负数从尾部算起
bool cs_mgr::insert_cmd(command * c, int g, int p)
{
	if (!adjust_pos(&g, cs_count())) return false;
	if (!adjust_pos(&p, cmd_count(g) + 1)) return false;
	command_set *cs = m_css[g];
	cs->cv.insert(cs->cv.begin() + p, c);
	return true;
}

// 删除程序,传入负数则从后算起
bool cs_mgr::delete_cmd(int g, int p)
{
	if (!adjust_pos(&g, cs_count())) return false;
	if (!adjust_pos(&p, cmd_count(g))) return false;
	command_set * cs = m_css[g];
	delete cs->cv[p];
	cs->cv.erase(cs->cv.begin() + p);
	return true;
}

// 返回组的总数
int cs_mgr::cs_count() const
{
	return static_cast<int>(m_css.size());
}

// 返回组内程序总数,传入负数则从后算起
int cs_mgr::cmd_count(int g) const
{
	if (!adjust_pos(&g, cs_count())) return -1;
	return static_cast<int>(m_css[g]->cv.size());
}

// 返回所有程序的总数
int cs_mgr::cmd_count() const
{
	int count = 0;
	for (int i = 0; i < cs_count(); i++)
	{
		count += cmd_count(i);
	}
	return count;
}

// 测试命令是否存在, 此函数没有回绕的说法
bool cs_mgr::cmd_exist(int g, int p) const
{
	if (g < 0 || g >= cs_count()) return false;
	if (p < 0 || p >= cmd_count(g)) return false;
	return true;
}

// 取指定程序, 传入负数时从尾部算起
command * cs_mgr::get_cmd(int g, int p) const
{
	if (!adjust_pos(&g, cs_count())) return NULL;
	if (!adjust_pos(&p, cmd_count(g))) return NULL;
	return m_css[g]->cv[p];
}

// 取指定组信息
info * cs_mgr::get_cs_info(int g) const
{
	if (!adjust_pos(&g, cs_count())) return NULL;
	return m_css[g]->si;
}
