#pragma once

#include "log.h"
#include <time.h>

namespace tp
{
	enum context_id
	{
		LCID_TIME = 11,
		LCID_TEXT = 12,
		LCID_TYPE = 13,
		LCID_INDENT = 14,
		LCID_TID = 15,
		LCID_PID = 16,
	};

	class tls_value
	{
	public:
		tls_value()	{ m_index = TlsAlloc();	}
		~tls_value(){ TlsFree(m_index);	}
		int get() const
		{
			return reinterpret_cast<int>(TlsGetValue(m_index));
		}
		bool set(int new_val)
		{
			return TlsSetValue(m_index, reinterpret_cast<LPVOID>(new_val)) == TRUE;
		}
	private:
		DWORD m_index;
	};

class lc_time : public log_context
{
public:
	lc_time(const wchar_t * time_fmt = NULL, bool show_millisec = false) : log_context(LCID_TIME)
	{
		if (!time_fmt) time_fmt = L"%H:%M:%S";

		m_time_fmt = time_fmt;
		m_show_millisec = show_millisec;
	}

	std::wstring value(unsigned int) const
	{
		wchar_t time_str[64] = {0};
		time_t ct = time(NULL);
		struct tm otm;
		localtime_s(&otm, &ct);
		size_t time_len = aw::strftime(time_str, sizeof(time_str)/sizeof(time_str[0]) - 1, m_time_fmt.c_str(), &otm);
		if (m_show_millisec)
		{
			SYSTEMTIME st;
			GetSystemTime(&st);
			aw::strncpy_s(time_str + time_len, 64 - time_len, cfmt<wchar_t>(L".%03d", st.wMilliseconds), _TRUNCATE);
			time_str[63] = '\0';
		}
		return time_str;
	}

private:
	std::wstring m_time_fmt;
	bool m_show_millisec;
	bool padding[3];
};

class lc_text : public log_context
{
public:
	lc_text(const wchar_t * text) : log_context(LCID_TEXT)
	{
		if (text) m_text = text;
	}
	std::wstring value(unsigned int) const
	{
		return m_text;
	}
private:
	std::wstring m_text;
};

class lc_type : public log_context
{
public:
	lc_type(const wchar_t * type_str) : log_context(LCID_TYPE)
	{
		if (type_str) m_type_str = type_str;
	}
	std::wstring value(unsigned int type) const
	{
		if (type < m_type_str.size())
		{
			return m_type_str.substr(type, 1) + L"|";
		}
		return L"|";
	}
private:
	std::wstring m_type_str;
};

class lc_indent : public log_context
{
private:

	static tls_value& tls_val()
	{
		static tls_value s_tv;
		return s_tv;
	}
public:
	lc_indent() : log_context(LCID_INDENT)
	{
	}
	std::wstring value(unsigned int) const
	{
		int l = tls_val().get();
		if (l > 0)
		{
			return std::wstring(static_cast<unsigned int>(l), L' ');
		}

		return L"";
	}
	static bool add_indent(int indent)
	{
		tls_value& tv = tls_val();
		return tv.set(tv.get() + indent);
	}
};

class lc_tid : public log_context
{
	typedef std::map<DWORD, std::wstring> tns_t;

public:
	lc_tid(const wchar_t * fmt = NULL) : log_context(LCID_TID)
	{
		if (!fmt) fmt = L"%04u";
		m_fmt = fmt;
	}
	std::wstring value(unsigned int) const
	{
		const tns_t& tns = get_tns();
		DWORD tid = GetCurrentThreadId();
		tns_t::const_iterator it = tns.find(tid);
		if (it != tns.end())
		{
			return it->second;
		}
		else
		{
			return (const wchar_t*)cz(m_fmt.c_str(), tid);
		}
	}

	static bool set_thread_name(DWORD tid, const wchar_t * name)
	{
		get_tns()[tid] = name;
		return true;
	}

	static bool set_thread_name(const wchar_t * name)
	{
		set_thread_name(GetCurrentThreadId(), name);
	}

private:
	std::wstring m_fmt;

	static tns_t& get_tns()
	{
		static tns_t s_tns;
		return s_tns;
	}
};

class lc_pid : public log_context
{
public:
	lc_pid(const wchar_t * fmt = NULL) : log_context(LCID_PID)
	{
		if (!fmt) fmt = L"%04u";
		m_str = (const wchar_t*)cz(fmt, GetCurrentProcessId());
	}
	std::wstring value(unsigned int) const
	{
		return m_str;
	}

private:
	std::wstring m_str;
	DWORD m_pid;
};

}