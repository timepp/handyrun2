#ifndef TP_OSS_WIN_H_INCLUDED
#define TP_OSS_WIN_H_INCLUDED

#include <windows.h>

typedef DWORD threadid_t;

namespace tp
{
	struct os
	{
		class tls_value
		{
		public:
			tls_value()	{ m_index = TlsAlloc();	}
			~tls_value(){ TlsFree(m_index);	}
			void* get() const
			{
				return TlsGetValue(m_index);
			}
			bool set(void* new_val)
			{
				return TlsSetValue(m_index, new_val) == TRUE;
			}
		private:
			DWORD m_index;
		};

		static threadid_t current_tid()
		{
			return static_cast<threadid_t>(GetCurrentThreadId());
		}
	};
}

#endif