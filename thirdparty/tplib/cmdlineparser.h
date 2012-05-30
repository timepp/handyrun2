#ifndef TP_CMDLINEPARSER_H_INCLUDED
#define TP_CMDLINEPARSER_H_INCLUDED

#include <string>
#include <list>
#include <map>
#include <vector>
#include "./auto_release.h"

namespace tp
{

class cmdline_parser
{

private:
	typedef std::map<std::wstring, std::wstring> strmap_t;
	typedef std::vector<std::wstring> strlist_t;

	strmap_t m_params;
	strlist_t m_targets;

public:
	bool parse(const wchar_t* cmd_line)
	{
		/** windows下对命令行的处理：参数中间有空格时，可以用引号引起来。这样就有一个如何表示引号的问题。
		 *  使用\"来表示"。\如果在引号之前，表示那是一个普通引号。\在其它位置保持其本意
		 */
		std::list<std::wstring> param_list;
		const wchar_t * p = cmd_line;
		while(*p)
		{
			bool in_quote = false;
			std::wstring param;
			const wchar_t *q = p;
			for(;;)
			{
				if (q[0]== L'\\' && q[1] == L'\"')
				{
					param += L'\"';
					q += 2;
					continue;
				}

				if (q[0] == L'\"')
				{
					in_quote = !in_quote;
					q++;
					continue;
				}

				if (q[0] == L' ' && !in_quote)
				{
					p = q+1;
					break;
				}

				if (q[0] == L'\0')
				{
					p = q;
					break;
				}

				param += *q;
				q++;
			}

			param_list.push_back(param);
		}

		size_t argc = param_list.size();
		const wchar_t** argv = new const wchar_t* [argc];
		ON_LEAVE_1(delete[] argv, const wchar_t**, argv);

		int i = 0;
		for (std::list<std::wstring>::const_iterator it = param_list.begin(); it != param_list.end(); ++it)
		{
			argv[i++] = it->c_str();
		}

		bool ret = parse(argc, argv);

		return ret;
	}

	bool parse(size_t argc, const wchar_t* const * argv)
	{
		if (!argv) return false;

		bool in_targets = false;
		for (size_t i = 1; i < argc; i++)
		{
			const wchar_t* arg = argv[i];
			if (!arg) return false;

			if (arg[0] != '-' || in_targets)
			{
				m_targets.push_back(arg);
				continue;
			}
			if (arg[1] == '-' && !arg[2])
			{
				in_targets = true;
				continue;
			}

			if (arg[1] == '-')
			{
				// 两个减号
				const wchar_t *p = wcschr(arg, L'=');
				if (p != NULL)
				{
					std::wstring param(arg + 2, static_cast<size_t>(p - arg - 2));
					m_params[param] = p + 1;
				}
				else
				{
					std::wstring param = arg + 2;
					if (i + 1 < argc && argv[i+1][0] != L'-')
					{
						m_params[param] = argv[i + 1];
						i++;
					}
					else
					{
						m_params[param] = L"";
					}
				}
			}
			else
			{
				// 一个减号
				for (const wchar_t * p = arg + 1; *p; p++)
				{
					std::wstring param = std::wstring(1, *p);
					if (!p[1] && i + 1 < argc && argv[i+1][0] != L'-')
					{
						m_params[param] = argv[i+1];
						i++;
					}
					else
					{
						m_params[param] = L"";
					}
				}
			}
		}

		return true;
	}

	std::wstring get_opt(const wchar_t * opt, const wchar_t * opt_alternate = 0)
	{
		if (!opt) return L"";

		strmap_t::const_iterator it = m_params.find(opt);
		if (it != m_params.end() && it->second.length() > 0)
		{
			return it->second;
		}

		return get_opt(opt_alternate);
	}
	int get_opt_as_int(const wchar_t* opt, const wchar_t * opt_alternate = 0)
	{
		std::wstring val = get_opt(opt, opt_alternate);
		return _wtoi(val.c_str());
	}
	bool opt_exist(const wchar_t* opt, const wchar_t * opt_alternate = 0)
	{
		if (!opt) return false;

		if (m_params.find(opt) != m_params.end())
		{
			return true;
		}

		return opt_exist(opt_alternate);
	}

	std::wstring format_params() const
	{
		std::wstring info;

		info += L"params:\n";
		for (strmap_t::const_iterator it = m_params.begin(); it != m_params.end(); ++it)
		{
			info += L"  ";
			info += it->first;
			info += L":";
			info += it->second;
			info += L"\n";
		}
		
		info += L"\ntargets:\n";
		for (strlist_t::const_iterator it = m_targets.begin(); it != m_targets.end(); ++it)
		{
			info += *it;
			info += L"\n";
		}

		return info;
	}

	std::wstring get_target(size_t index) const
	{
		if (index >= m_targets.size())
		{
			return L"";
		}
		return m_targets[index];
	}

	size_t get_target_count() const
	{
		return m_targets.size();
	}

	bool is_no_param() const
	{
		return m_targets.size() == 0 && m_params.size() == 0;
	}
};

}

#endif