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
	struct exception
	{
		std::wstring oplist;
		std::wstring message;

		static exception fromWinError(DWORD e) {
			return { CURRENT_OPLIST(), std::wstring(tp::edwin(e)) };
		}
		static exception fromStdError(int e) {
			return { CURRENT_OPLIST(), std::wstring(tp::edstd(e)) };
		}
		static exception fromHRESULT(HRESULT e) {
			return { CURRENT_OPLIST(), std::wstring(tp::edcom(e)) };
		}
		static exception fromString(const wchar_t* e) {
			return { CURRENT_OPLIST(), std::wstring(e) };
		}
	};

	inline void throw_winerr_when(bool cond)
	{
		if (cond) 
		{
			throw exception::fromWinError(GetLastError());
		}
	}
	inline void throw_if_lasterror(LONG err)
	{
		if (err != 0)
		{
			throw exception::fromWinError(err);
		}
	}
	inline void throw_stderr_when(bool cond)
	{
		if (cond)
		{
			throw exception::fromStdError(errno);
		}
	}
	inline void throw_when_fail(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw exception::fromHRESULT(hr);
		}
	}
	inline void throw_when(bool cond, const wchar_t* msg)
	{
		if (cond)
		{
			throw exception::fromString(msg);
		}
	}
	
}

#endif