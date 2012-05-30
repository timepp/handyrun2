#pragma once

#include "log.h"
#include <windows.h>
#include <stdio.h>
#include <string>

namespace tp
{

class ld_console : public log_device
{
public:
	ld_console()
		: m_free_console_on_close(false)
		, m_handle(NULL)
		, m_default_attr(0)
	{
	}

	virtual bool open()
	{
		if (::AllocConsole())
		{
			m_free_console_on_close = true;
		}
		m_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (is_handle_valid(m_handle))
		{
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			GetConsoleScreenBufferInfo(m_handle, &csbi);
			m_default_attr = csbi.wAttributes;
			return true;
		}

		return false;
	}
	virtual bool close()
	{
		if (m_free_console_on_close)
		{
			::FreeConsole();
			m_handle = NULL;
		}
		return true;
	}
	virtual bool flush()
	{
		return true;
	}

	virtual size_t write(const wchar_t * buf, size_t len, int context_id)
	{
		if (!is_handle_valid(m_handle)) return 0;
		WORD attr = m_default_attr;
		if (m_ca.find(context_id) != m_ca.end())
		{
			attr = m_ca[context_id];
		}
		::SetConsoleTextAttribute(m_handle, attr);
		DWORD wrote;
		::WriteConsoleW(m_handle, buf, static_cast<DWORD>(len), &wrote, NULL);
		return static_cast<size_t>(wrote);
	}

	void set_context_attr(int context_id, WORD attr)
	{
		m_ca[context_id] = attr;
	}

protected:
	HANDLE m_handle;
	std::map<int, WORD> m_ca;
	WORD m_default_attr;
	bool m_free_console_on_close;
	bool padding[1];

private:
	static bool is_handle_valid(HANDLE handle)
	{
		return (handle != NULL && handle != INVALID_HANDLE_VALUE);
	}
};

class ld_mem_log : public log_device
{
public:
	ld_mem_log()
	{
		m_log.reserve(1000 * 1000);
	}

	virtual bool open() { return true; }
	virtual bool close() { return true; }
	virtual size_t write(const wchar_t * buf, size_t len, int)
	{
		m_log += std::wstring(buf, len);
		return len;
	}
	virtual bool flush()
	{
		return true;
	}

	void get_log(std::wstring& str)
	{
		str = m_log;
	}

private:
	std::wstring m_log;
};

class ld_debug_output : public log_device
{
public:
	ld_debug_output()
	{
	}

	virtual bool open()
	{
		return true;
	}

	virtual bool close()
	{
		return true;
	}

	virtual size_t write(const wchar_t * buf, size_t len, int)
	{
		std::wstring str(buf, len);
		::OutputDebugStringW(str.c_str());
		return len;
	}

	virtual bool flush()
	{
		return true;
	}
};

class ld_file : public log_device
{
public:
	ld_file(const wchar_t * filename)
	{
		m_filename = filename;
		m_fp = NULL;
	}
	
	virtual bool open()
	{
		m_fp = _wfsopen(m_filename.c_str(), L"wt", _SH_DENYWR);
		return (m_fp != NULL);
	}

	virtual bool close()
	{
		if (m_fp)
		{
			fclose(m_fp);
			m_fp = NULL;
			return true;
		}
		return false;
	}

	virtual size_t write(const wchar_t *buf, size_t len, int)
	{
		if (m_fp) return static_cast<size_t>(fwprintf(m_fp, L"%.*s", len, buf));
		return 0;
	}

	virtual bool flush()
	{
		return fflush(m_fp) == 0;
	}

protected:
	FILE * m_fp;
	std::wstring m_filename;
};
/*
class ld_xml_file : public ld_file
{
};
*/

} // namespace tp
