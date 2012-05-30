#ifndef TP_OPBLOCK_H_INCLUDED
#define TP_OPBLOCK_H_INCLUDED

#include <map>
#include <list>
#include <string>
#include "oss.h"
#include "service.h"
#include "defs.h"

#define SETOP(x) tp::service<tp::opmgr>::instance().set_op(x)
#define OPBLOCK(x) SETOP(L"");tp::opblock CONCAT(opblk,__LINE__)(x)
#define CURRENT_OPLIST() tp::service<tp::opmgr>::instance().get_oplist(L" -> ")

namespace tp
{
	class opmgr
	{
		typedef std::list<std::wstring> strlist_t;
		typedef std::map<threadid_t, strlist_t*> opmap_t;
	public:
		~opmgr()
		{
			free();
		}

		void push_block(const std::wstring& op)
		{
			strlist_t*& oplist = m_obmap[os::current_tid()];
			if (!oplist)
			{
				oplist = new strlist_t;
			}
			oplist->push_back(op);
		}
		void pop_block()
		{
			strlist_t* oplist = m_obmap[os::current_tid()];
			if (oplist)
			{
				oplist->pop_back();
			}
		}
		void set_op(const std::wstring& op)
		{
			m_op = op;
		}
		std::wstring get_oplist(const std::wstring& sep) const
		{
			std::wstring lstr;
			opmap_t::const_iterator it = m_obmap.find(os::current_tid());
			if (it != m_obmap.end())
			{
				const strlist_t* oplist = it->second;
				if (oplist)
				{
					for (strlist_t::const_iterator it = oplist->begin(); it != oplist->end(); ++it)
					{
						if (!lstr.empty()) lstr += sep;
						lstr += *it;
					}
				}
			}

			if (!m_op.empty())
			{
				if (!lstr.empty()) lstr += sep;
				lstr += m_op;
			}
			return lstr;
		}

	private:
		opmap_t m_obmap;
		std::wstring m_op;

		void free()
		{
			for (opmap_t::const_iterator it = m_obmap.begin(); it != m_obmap.end(); ++it)
			{
				delete it->second;
			}
			m_obmap.clear();
		}
	};

	class opblock
	{
	public:
		opblock(const std::wstring& op)
		{
			tp::service<opmgr>::instance().push_block(op);
		}
		~opblock()
		{
			tp::service<opmgr>::instance().pop_block();
		}
	};
}

#endif