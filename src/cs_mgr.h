#pragma once

#include "prog.h"

class cs_mgr
{
public:
	cs_mgr();
	~cs_mgr();

	// 命令集
	bool insert_cs(info * cs_info, int g = -1);
	bool delete_cs(int g);
	// 命令
	bool insert_cmd(command * c, int g, int p = -1);
	bool delete_cmd(int g, int p);

	void clear();

	// 计数
	int cs_count() const;
	int cmd_count() const;
	int cmd_count(int g) const;

	// 测试程序是否存在
	bool cmd_exist(int g, int p) const;
	// 取指定程序
	command * get_cmd(int g, int p) const;
	// 取指定组信息(组在外界是看不到的,只能看到组信息)
	info * get_cs_info(int g) const;

private:
	cs_mgr(const cs_mgr&);

	std::vector<command_set *> m_css;
};