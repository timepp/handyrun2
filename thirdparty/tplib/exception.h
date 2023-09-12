#ifndef TP_EXCEPTION_H_INCLUDED
#define TP_EXCEPTION_H_INCLUDED

#include "format_shim.h"
#include "opblock.h"
#include <memory>

#ifdef NE
#pragma warning("conflict: NE already defined")
#else
#define NE(statment1, statment2) try { statment1; } catch (...) { statment2; }
#endif

namespace tp
{
	struct error
	{
		virtual std::wstring desc() const = 0;
		virtual ~error() {}
	};
	struct error_win : public error
	{
		error_win(DWORD e) : errcode(e) {}
		DWORD errcode;
		virtual std::wstring desc() const { return std::wstring(tp::edwin(errcode)); }
	};
	struct error_std : public error
	{
		error_std(int e) : errcode(e) {}
		int errcode;
		virtual std::wstring desc() const { return std::wstring(tp::edstd(errcode)); }
	};
	struct error_com : public error
	{
		error_com(HRESULT e) : errcode(e) {}
		HRESULT errcode;
		virtual std::wstring desc() const { return std::wstring(tp::edcom(errcode)); }
	};

	struct error_custom : public error
	{
		error_custom(const wchar_t* msg) : message(msg) {}
		std::wstring message;
		virtual std::wstring desc() const { return message; }
	};


	struct exception
	{
		std::wstring oplist;
		std::wstring message;
		std::unique_ptr<tp::error> err;

		explicit exception(error* e, const wchar_t* msg = 0) : err(e), oplist(CURRENT_OPLIST())
		{
			if (msg) message = msg;
		}
	};

	inline void throw_winerr_when(bool cond)
	{
		if (cond) 
		{
			throw tp::exception(new tp::error_win(GetLastError()));
		}
	}
	inline void throw_if_lasterror(LONG err)
	{
		if (err != 0)
		{
			throw tp::exception(new tp::error_win(err));
		}
	}
	inline void throw_stderr_when(bool cond)
	{
		if (cond)
		{
			throw tp::exception(new tp::error_std(errno));
		}
	}
	inline void throw_when_fail(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw tp::exception(new tp::error_com(hr));
		}
	}
	inline void throw_when(bool cond, const wchar_t* msg)
	{
		if (cond)
		{
			throw tp::exception(new tp::error_custom(msg));
		}
	}
	
}

#endif