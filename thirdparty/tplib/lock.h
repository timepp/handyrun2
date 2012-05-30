#ifndef TP_LOG_H_INCLUDED
#define TP_LOG_H_INCLUDED

namespace tp
{


struct dummy_lock
{
	void lock() {}
	void unlock() {}
};

struct critical_section_lock
{
	critical_section_lock()  { ::InitializeCriticalSection(&m_cs); }
	~critical_section_lock() { ::DeleteCriticalSection(&m_cs); }
	void lock()   {::EnterCriticalSection(&m_cs); }
	void unlock() {::LeaveCriticalSection(&m_cs); }
private:
	CRITICAL_SECTION m_cs;
};

template <typename T>
class autolocker
{
public:

	explicit autolocker(T& l) : locker(l)
	{
		locker.lock();
	}

	~autolocker()
	{
		locker.unlock();
	}

private:

	autolocker& operator= (const autolocker& a);

	T& locker;
};


}

#endif