#pragma once

#include <vector>
#include <string>
#include <windows.h>

// ��������: ֻ����һ��·����Ϣ
struct command
{
	command();
	command(const command& c);
	virtual ~command(){}
	virtual command * clone() const;         // prototype

	std::wstring path;                   // ·��(���������·��)
};

// ������Ϣ
struct info
{
	virtual ~info(){}
};

// ���, �����Ա����ȫ����Ȩ
struct command_set
{
	~command_set();

	info * si;                          // �����Ϣ
	std::vector<command *> cv;              // ���
};

// ͼ������
struct icon_desc
{
	std::wstring file;
	int index;
};

// ���򣺰�����������ص�������Ϣ
struct prog : public command
{
	prog();
	prog(const prog& p);
	virtual command * clone() const;

	std::wstring param;                  // ���в���
	std::wstring drop_param;             // �ϷŲ���
	std::wstring work_dir;               // ����Ŀ¼
	std::wstring comment;                // ע��
	icon_desc icon;                      // ͼ��
	int show_cmd;                        // ��ʾ��ʽ(��С��,���...)
};


// ������Ϣ
struct group_info : public info
{
	group_info(const wchar_t * n = NULL, COLORREF cr = 0xFF000000);

	std::wstring name;                   // ������
	COLORREF color;                      // ����ɫ
};

// ������Ϣ
struct index_info : public info
{
	index_info();

	std::wstring path;
	int depth;
	icon_desc icon;
};
