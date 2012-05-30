#ifndef TP_API_WRAPPER_H_INCLUDED
#define TP_API_WRAPPER_H_INCLUDED

#include <cstdio>
#include <cstring>
#include <windows.h>
#include <time.h>
#include <stdarg.h>

/** 以同样的函数名封装操作系统或C++库提供的字符串相关函数的ansi和unicode版本，从而可以应用到模板中
 */

namespace tp
{
	struct aw
	{
		static int snprintf_s(char * buf, size_t len, const char * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			return vsnprintf_s(buf, len, fmt, args);
		}
		static int snprintf_s(wchar_t * buf, size_t len, const wchar_t * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			return vsnprintf_s(buf, len, fmt, args);
		}

		static int vsnprintf_s(char * buf, size_t len, const char *fmt, va_list arg)
		{
			return ::vsnprintf_s(buf, len, _TRUNCATE, fmt, arg);
		}
		static int vsnprintf_s(wchar_t * buf, size_t len, const wchar_t * fmt, va_list arg)
		{
			return ::_vsnwprintf_s(buf, len, _TRUNCATE, fmt, arg);
		}
		static int _vscprintf(const char * fmt, va_list arg)
		{
			return ::_vscprintf(fmt, arg);
		}
		static int _vscprintf(const wchar_t * fmt, va_list arg)
		{
			return ::_vscwprintf(fmt, arg);
		}
		static int strerror_s(char *buf, size_t len, int err_num)
		{
			return ::strerror_s(buf, len, err_num);
		}
		static int strerror_s(wchar_t *buf, size_t len, int err_num)
		{
			return ::_wcserror_s(buf, len, err_num);
		}
		static errno_t strncpy_s(char *dest, size_t len, const char * src, size_t max_count)
		{
			return ::strncpy_s(dest, len, src, max_count);
		}
		static errno_t strncpy_s(wchar_t *dest, size_t len, const wchar_t * src, size_t max_count)
		{
			return ::wcsncpy_s(dest, len, src, max_count);
		}

		static size_t strftime(char * buf, size_t len, const char * fmt, const tm * t)
		{
			return ::strftime(buf, len, fmt, t);
		}
		static size_t strftime(wchar_t * buf, size_t len, const wchar_t * fmt, const tm * t)
		{
			return ::wcsftime(buf, len, fmt, t);
		}


		static DWORD FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, char * lpBuffer, DWORD nSize, va_list* Arguments)
		{
			return ::FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
		}
		static DWORD FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, wchar_t * lpBuffer, DWORD nSize, va_list* Arguments)
		{
			return ::FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
		}
	};
}

#endif