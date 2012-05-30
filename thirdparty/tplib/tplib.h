#pragma once

#include "format_shim.h"
#include "log.h"
#include "log_device.h"
#include "log_context.h"
#include "auto_release.h"
#include "algorithm.h"

namespace tp
{
	// ∏®÷˙¿‡
	class log_indenter
	{
	public:
		log_indenter(int indent) : m_indent(indent)
		{
			lc_indent::add_indent(m_indent);
		}
		~log_indenter()
		{
			lc_indent::add_indent(-m_indent);
		}
	private:
		int m_indent;
	};

	class log_cc
	{
	public:
		log_cc(log_device * ld) : m_ld(ld) {}
		const log_cc& operator << (log_context * lc) const
		{
			log_add_context(m_ld, lc);
			return *this;
		}
		const log_cc& operator << (const wchar_t * text) const
		{
			*this << new lc_text(text);
			return *this;
		}
	private:
		log_device * m_ld;
	};


	namespace sc
	{
		inline void log_default_console_config(bool show_tid = true)
		{
			ld_console * c = new ld_console;
			c->set_context_attr(LCID_TIME, FOREGROUND_GREEN);
			c->set_context_attr(LCID_TYPE, FOREGROUND_RED);
			c->set_context_attr(LCID_TID,  FOREGROUND_GREEN | FOREGROUND_BLUE);
			log_add_device(c, 0xFF);
			log_cc(c) << new lc_time(L"%H:%M:%S", true);
			if (show_tid) log_cc(c) << L" " << new lc_tid;
			log_cc(c) << L" " << new lc_type(L"IVDE") << new lc_indent;
		}
		inline void log_default_file_config(const wchar_t * prefix)
		{
			ld_file * f = new ld_file(tp::cz(L"%s%s.log", prefix, lc_pid().value(0).c_str()));
			log_add_device(f, 0xFF);
			log_cc(f) << new lc_time(L"%Y-%m-%d %H:%M:%S", true) << L" " << new lc_tid << L" " << new lc_type(L"IVDE") << new lc_indent;
		}
		inline void log_default_mem_config(ld_mem_log& m)
		{
			log_add_device(&m, 0xFF, false);
			log_cc(&m) << new lc_time(L"%Y-%m-%d %H:%M:%S", true) << L" " << new lc_tid << L" " << new lc_type(L"VIDE") << new lc_indent;
		}
		inline void log_win_error(const wchar_t * prefix)
		{
			tp::log(tp::cz(L"%s: %s", prefix, &tp::edwin()));
		}
	}

}
