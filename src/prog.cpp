#include "stdafx.h"
#include "prog.h"
#include "global_data.h"

command::command()
{
}
command::command(const command &c) : path(c.path)
{
}

command * command::clone() const
{
	return new command(*this);
}

command_set::~command_set()
{
	for (std::vector<command *>::const_iterator ci = cv.begin();
	     ci != cv.end();
	     ++ci)
	{
		delete *ci;
	}
	cv.clear();
	delete si;
	si = NULL;
}

prog::prog() : show_cmd(SW_SHOW)
{
	icon.index = 0;
}
prog::prog(const prog& p)
: command(p), 
param(p.param), work_dir(p.work_dir), show_cmd(p.show_cmd), 
icon(p.icon)
{
}

command * prog::clone() const
{
	return new prog(*this);
}

group_info::group_info(const wchar_t * n, COLORREF cr) :
	name(n? n : sr::default_grp_name), color(cr)
{
}

index_info::index_info() : depth(1)
{
	icon.index = 0;
}
