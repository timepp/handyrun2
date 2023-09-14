#pragma once

#include "defs.h"

class hotkey_waker : public CMessageFilter
{
	typedef void (*wake_cmd_ptr_t)();
public:
	static hotkey_waker * instance();
	// 注册热键, wake_cmd为激活函数. 为空时保持原来的激活函数
	bool register_hotkey(const wchar_t * hotkey, wake_cmd_ptr_t wc = NULL);

	hotkey_waker();
	virtual ~hotkey_waker();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
	hotkey m_hotkey;
	wake_cmd_ptr_t m_wc;
	int m_hotkey_id;

	HHOOK m_kb_hook;

	void clean_hotkey();
	static hotkey translate_hotkey(const wchar_t * hk);
	static UINT get_vk_code(const wchar_t * vk);
	static LRESULT CALLBACK ll_kb_proc(int code, WPARAM wp, LPARAM lp);
	static void ll_kb_proc_internal(const KBDLLHOOKSTRUCT * kb, UINT msg);
};

