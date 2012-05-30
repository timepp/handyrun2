#pragma once

#include <windows.h>

class handyrun_err
{
public:
	virtual const wchar_t * what() = 0;
};

enum err_source { win, dos};

class os_err : public handyrun_err
{
public:
	os_err(err_source s, const wchar_t * op = 0);
	os_err(err_source s, const wchar_t * op, const wchar_t * fmt, ...);
	virtual const wchar_t * what();
	err_source m_es;
	int m_err_code;

private:
	void save_error(err_source s);
	void form_err_msg(const wchar_t * op, const wchar_t * extra);
	bool get_err_desc(wchar_t * buf, size_t buf_len);

	wchar_t m_err_msg[4096];
};

