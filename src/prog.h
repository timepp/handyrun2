#pragma once

#include <vector>
#include <string>
#include <windows.h>

// 基本命令: 只包含一个路径信息
struct command
{
	command();
	command(const command& c);
	virtual ~command(){}
	virtual command * clone() const;         // prototype

	std::wstring path;                   // 路径(可能是相对路径)
};

// 基本信息
struct info
{
	virtual ~info(){}
};

// 命令集, 对其成员有完全所有权
struct command_set
{
	~command_set();

	info * si;                          // 命令集信息
	std::vector<command *> cv;              // 命令集
};

// 图标描述
struct icon_desc
{
	std::wstring file;
	int index;
};

// 程序：包含和运行相关的相它信息
struct prog : public command
{
	prog();
	prog(const prog& p);
	virtual command * clone() const;

	std::wstring param;                  // 运行参数
	std::wstring drop_param;             // 拖放参数
	std::wstring work_dir;               // 工作目录
	std::wstring comment;                // 注释
	icon_desc icon;                      // 图标
	int show_cmd;                        // 显示方式(最小化,最大化...)
};


// 分组信息
struct group_info : public info
{
	group_info(const wchar_t * n = NULL, COLORREF cr = 0xFF000000);

	std::wstring name;                   // 组名字
	COLORREF color;                      // 组颜色
};

// 索引信息
struct index_info : public info
{
	index_info();

	std::wstring path;
	int depth;
	icon_desc icon;
};
