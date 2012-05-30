#include "stdafx.h"
#include "error.h"
#include "defs.h"

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

os_err::os_err(err_source s, const wchar_t * op)
{
	save_error(s);
	form_err_msg(op, 0);
}

os_err::os_err(err_source s, const wchar_t * op, const wchar_t * fmt, ...)
{
	save_error(s);

	wchar_t extra_info[1024];
	va_list args;
	va_start(args, fmt);
	vswprintf_s(extra_info, elem_of(extra_info), fmt, args);
	va_end(args);

	form_err_msg(op, extra_info);
}

const wchar_t * os_err::what()
{
	return m_err_msg;
}

void os_err::save_error(err_source s)
{
	m_es = s;
	if (s == win)
	{
		m_err_code = static_cast<int>(GetLastError());
	}
	else
	{
		m_err_code = errno;
	}
}

bool os_err::get_err_desc(wchar_t *buf, size_t buf_len)
{
	if (m_err_code == 0) return false;

	if (m_es == win)
	{
		return FormatMessage(
			FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, static_cast<DWORD>(m_err_code), 0,
			buf, buf_len, NULL) != 0;
	}
	else
	{
		return _wcserror_s(buf, buf_len, m_err_code) == 0;
	}
}

void os_err::form_err_msg(const wchar_t *op, const wchar_t *extra)
{
	wchar_t err_desc_buf[1024];
	wchar_t err_title[1024];

	wchar_t pad1[] = {L'\n', L'\n', L'\0'};
	const wchar_t * extra_title = L"����Ĳ�����Ϣ:\n";

	// ��һ����, ������Ϣ
	if (!op)
	{
		_snwprintf_s(err_title, elem_of(err_title), _TRUNCATE,
			L"%s��������", hc::handyrun_name);
	}
	else
	{
		_snwprintf_s(err_title, elem_of(err_title), _TRUNCATE, 
			L"%s��%sʱ��������", hc::handyrun_name, op);
	}
	
	// �ڶ�����:����ϵͳ��������
	if (!get_err_desc(err_desc_buf, elem_of(err_desc_buf)))
	{
		_snwprintf_s(err_desc_buf, elem_of(err_desc_buf), _TRUNCATE,
			L"�������: %d", m_err_code);
	}

	// ��������: ����Ķ������
	if (!extra)
	{
		pad1[0] = L'\0';
		extra = hc::empty_str;
		extra_title = hc::empty_str;
	}

	// �����һ��
	_snwprintf_s(m_err_msg, elem_of(m_err_msg), _TRUNCATE,
		L"%s:\n\n"
		L"  %s%s"
		L"%s%s", err_title, err_desc_buf, pad1, extra_title, extra);
}

