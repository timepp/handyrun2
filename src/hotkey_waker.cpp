#include "stdafx.h"
#include "hotkey_waker.h"
#include "error.h"
#include "helper.h"
#include "global_data.h"
#include "log.h"

hotkey_waker::hotkey_waker() 
: m_hotkey(), m_wc(NULL), m_hotkey_id(1200), m_kb_hook(NULL)
{
}

hotkey_waker::~hotkey_waker()
{
	clean_hotkey();
}

void hotkey_waker::clean_hotkey()
{
	UnregisterHotKey(NULL, m_hotkey_id);
	if (m_kb_hook != NULL)
	{
		UnhookWindowsHookEx(m_kb_hook);
		m_kb_hook = NULL;
	}
}

hotkey_waker * hotkey_waker::instance()
{
	static hotkey_waker hw;
	return &hw;
}

bool hotkey_waker::register_hotkey(const wchar_t * hotkey, wake_cmd_ptr_t wc)
{
	log_msg(L"registering hotkey...");

	const wchar_t * op = L"注册激活热键";

	if (!hotkey)
	{
		clean_hotkey();
		return true;
	}

	m_hotkey = translate_hotkey(hotkey);
	if (wc) m_wc = wc;
	m_hotkey_id++;

	if (m_hotkey.fk == 0)
	{
		m_kb_hook = SetWindowsHookEx(WH_KEYBOARD_LL, ll_kb_proc, GetModuleHandleW(NULL), 0);
		if (m_kb_hook == NULL)
		{
			throw os_err(win, op);
		}
	}
	else
	{
		if (!RegisterHotKey(NULL, m_hotkey_id, m_hotkey.fk, m_hotkey.vk))
		{
			throw os_err(win, op);
		}
	}

	return true;
}

UINT hotkey_waker::get_vk_code(const wchar_t * vk)
{
	log_msg(L"loading vk-code...");

	const wchar_t * op = L"获取按键代码";

	tp::cz fn(L"%s\\%s", gd::hr_path.c_str(), hc::fn_vkcode);

	FILE * fp;
	if (_wfopen_s(&fp, fn, L"r, ccs=UNICODE") != 0)
	{
		throw os_err(dos, op, L"文件名:%s", &fn);
	}
	ON_LEAVE_1(fclose(fp), FILE*, fp);

	size_t key_len = wcslen(vk);
	wchar_t line[256];
	while (fgetws(line, elem_of(line), fp) != NULL)
	{
		const wchar_t * p = wcschr(line, L' ');
		if (p && p == line + key_len && _wcsnicmp(line, vk, key_len) == 0)
		{
			const wchar_t * q = wcschr(p, L'x');
			if (q)
			{
				return hlp::hex_val(q+1, 2);
			}
		}
	}
	return 0;
}

hotkey hotkey_waker::translate_hotkey(const wchar_t * hk)
{
	hotkey key = {0, 0};
	if (hk[0] == L'\"')
	{
		key.vk = get_vk_code(hk+1);
	}
	else
	{
		for (; *hk && !key.vk; hk++)
		{
			switch (*hk)
			{
			case L'@': key.fk |= MOD_ALT; break;
			case L'^': key.fk |= MOD_CONTROL; break;
			case L'_': key.fk |= MOD_SHIFT; break;
			case L'~': key.fk |= MOD_WIN; break;
			default: key.vk = get_vk_code(hk);
			}
		}
	}

	return key;
}

LRESULT hotkey_waker::ll_kb_proc(int code, WPARAM wp, LPARAM lp)
{
	if (code >= 0)
	{
		ll_kb_proc_internal((KBDLLHOOKSTRUCT *)lp, wp);
	}
	return CallNextHookEx(hotkey_waker::instance()->m_kb_hook, code, wp, lp);
}

void hotkey_waker::ll_kb_proc_internal(const KBDLLHOOKSTRUCT *kb, UINT msg)
{
	static DWORD prev_key_time = 0;
	const int double_press_interval = 500;
	hotkey_waker * hw = hotkey_waker::instance();

	bool key_equal = 
		kb->vkCode == hw->m_hotkey.vk ||
		hw->m_hotkey.vk == VK_CONTROL && (kb->vkCode == VK_LCONTROL || kb->vkCode == VK_RCONTROL) ||
		hw->m_hotkey.vk == VK_MENU && (kb->vkCode == VK_LMENU || kb->vkCode == VK_RMENU) ||
		hw->m_hotkey.vk == VK_SHIFT && (kb->vkCode == VK_LSHIFT || kb->vkCode == VK_RSHIFT);
	if (!key_equal)
	{
		// 两次ctrl中间夹杂其它的键，则程序不应被唤醒
		// 通过在其它键到来时设置g_prev_key_time为零解决上述问题
		prev_key_time = 0;
	}
	else if (msg == WM_KEYUP || msg == WM_SYSKEYUP)
	{
		if (kb->time - prev_key_time <= double_press_interval)
		{
			log_msg(L"double-pressed hotkey triggered");
			if (hw->m_wc) 
			{
				(*hw->m_wc)();
			}
			prev_key_time = 0;
		}
		else 
		{
			prev_key_time = kb->time;
		}
	}
}

BOOL hotkey_waker::PreTranslateMessage(MSG *msg)
{
	if (msg->hwnd == NULL && 
		msg->message == WM_HOTKEY && 
		msg->wParam == static_cast<WPARAM>(m_hotkey_id))
	{
		log_msg(L"normal hotkey triggered");
		if (m_wc) m_wc();
		return TRUE;
	}
	return FALSE;
}

