#ifndef TP_TSTRING_H_INCLUDED
#define TP_TSTRING_H_INCLUDED

/** tstring 扮演了一个“万用”字符串的角色，可以以ANSI或UNICODE构造，然后以ANSI或UNICODE取
 *          当构造的字符集和取的字符集不同时，会自动进行转换
 */

#include <string>

namespace tp
{
	class tstring
	{
	public:
		tstring(){}
		tstring(const char * str) : m_str_a(str)
		{
		}
		tstring(const wchar_t * str) : m_str_w(str) 
		{
		}
		void assign(const char * str)
		{
			m_str_a = str;
			m_str_w.clear();
		}
		void assign(const wchar_t * str)
		{
			m_str_w = str;
			m_str_a.clear();
		}

		operator const wchar_t * () const 
		{
			if (m_str_w.empty() && !m_str_a.empty())
			{
				size_t len;
				if (mbstowcs_s(&len, NULL, 0, m_str_a.c_str(), m_str_a.length()) == 0)
				{
					wchar_t * buf = new wchar_t[len];
					mbstowcs_s(&len, buf, len, m_str_a.c_str(), m_str_a.length());
					m_str_w.assign(buf, len);
					delete [] buf;
				}
			}
			return m_str_w.c_str();
		}

		operator const char * () const
		{
			if (m_str_a.empty() && !m_str_w.empty())
			{
				size_t len;
				if (wcstombs_s(&len, NULL, 0, m_str_w.c_str(), m_str_w.length()) == 0)
				{
					char * buf = new char[len];
					wcstombs_s(&len, buf, len, m_str_w.c_str(), m_str_w.length());
					m_str_a.assign(buf, len);
					delete [] buf;
				}
			}
			return m_str_a.c_str();
		}


	private:
		mutable std::string  m_str_a;
		mutable std::wstring m_str_w;
	};
}

#endif
