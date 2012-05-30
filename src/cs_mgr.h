#pragma once

#include "prog.h"

class cs_mgr
{
public:
	cs_mgr();
	~cs_mgr();

	// ���
	bool insert_cs(info * cs_info, int g = -1);
	bool delete_cs(int g);
	// ����
	bool insert_cmd(command * c, int g, int p = -1);
	bool delete_cmd(int g, int p);

	void clear();

	// ����
	int cs_count() const;
	int cmd_count() const;
	int cmd_count(int g) const;

	// ���Գ����Ƿ����
	bool cmd_exist(int g, int p) const;
	// ȡָ������
	command * get_cmd(int g, int p) const;
	// ȡָ������Ϣ(��������ǿ�������,ֻ�ܿ�������Ϣ)
	info * get_cs_info(int g) const;

private:
	cs_mgr(const cs_mgr&);

	std::vector<command_set *> m_css;
};