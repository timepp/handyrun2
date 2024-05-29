#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>

class slog
{
public:
	static slog * instance()
	{
		static slog l;
		return &l;
	}
	~slog()
	{
		if (m_fp != NULL)
		{
			fclose(m_fp);
			m_fp = NULL;
		}
	}

	void log_msg(const wchar_t * msg)
	{
		if (m_fp)
		{
			fwprintf(m_fp, L"%s  %s\n", format_curtime(L"%Y-%m-%d %H:%M:%S"), msg);
			fflush(m_fp);
		}
	}

private:
	FILE * m_fp;

	slog() : m_fp(NULL)
	{
		open_file();
	}

	bool open_file()
	{
		wchar_t path[MAX_PATH];
		wchar_t fn[MAX_PATH];
		GetModuleFileName(NULL, path, MAX_PATH);
		wchar_t *p = wcsrchr(path, L'\\');
		if (p == NULL) return false;
		*p = L'\0';
		_snwprintf_s(fn, MAX_PATH, _TRUNCATE, L"%s\\%s", path, L"handyrun.log");
		return _wfopen_s(&m_fp, fn, L"w") == 0;
	}

	const wchar_t * format_curtime(const wchar_t * fmt)
	{
		static wchar_t timestr[256];
		time_t ct = time(NULL);
		struct tm t;
		localtime_s(&t, &ct);
		wcsftime(timestr, 255, fmt, &t);
		return timestr;
	}
};

void log_msg(const wchar_t * msg)
{
#ifdef NEED_LOG
	slog::instance()->log_msg(msg);
#else
	(msg);
#endif
}