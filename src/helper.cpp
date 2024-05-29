#include "stdafx.h"
#include "helper.h"
#include "defs.h"

#include <sys/stat.h>
#include <stdarg.h>
#include <winternl.h>
#include <shlobj.h>

#include <algorithm>
#include <fstream>

namespace hlp
{

bool is_space(wchar_t c)
{
	return c == L' ' || c == L'\t';
}

int hex_val(wchar_t ch)
{
	if (ch <= L'9') return ch - L'0';
	if (ch <= L'F') return ch - L'A' + 10;
	return ch - L'a' + 10;
}

int hex_val(const wchar_t * ch, int n)
{
	int val = 0;
	for (; n > 0; ch++, n--)
	{
		val = val * 16 + hex_val(*ch);
	}
	return val;
}

COLORREF mid_color(COLORREF cr1, COLORREF cr2, double p)
{
	int r[] = {GetRValue(cr1), GetRValue(cr2)};
	int g[] = {GetGValue(cr1), GetGValue(cr2)};
	int b[] = {GetBValue(cr1), GetBValue(cr2)};
	return RGB(r[0] + p * (r[1] - r[0]), 
	           g[0] + p * (g[1] - g[0]), 
	           b[0] + p * (b[1] - b[0]));
}

void rect_ensure_inside(RECT *rc, RECT rc_out)
{
	if (rc->right > rc_out.right) ::OffsetRect(rc, rc_out.right - rc->right, 0);
	if (rc->bottom > rc_out.bottom) ::OffsetRect(rc, 0, rc_out.bottom - rc->bottom);
	if (rc->left < rc_out.left) ::OffsetRect(rc, rc_out.left - rc->left, 0);
	if (rc->top < rc_out.top) ::OffsetRect(rc, 0, rc_out.top - rc->top);
}

edge_type rect_dock(RECT *rc, RECT rc_out)
{
	rect_ensure_inside(rc, rc_out);
	int d[4] = {
		abs(rc->top - rc_out.top), abs(rc->left - rc_out.left), 
		abs(rc_out.right - rc->right), abs(rc_out.bottom - rc->bottom)
	};
	int d_min = *std::min_element(d, d+4);

	/**/ if (d_min == d[0]) { ::OffsetRect(rc, 0, -d[0]); return et_t; }
	else if (d_min == d[1]) { ::OffsetRect(rc, -d[1], 0); return et_l; }
	else if (d_min == d[2]) { ::OffsetRect(rc, d[2], 0);  return et_r; }
	else if (d_min == d[3]) { ::OffsetRect(rc, 0, d[3]);  return et_b; }
	else return et_t;
}

void draw_line(HDC hdc, int x1, int y1, int x2, int y2, COLORREF cr)
{
	HPEN pen = ::CreatePen(PS_SOLID, 1, cr);
	HPEN oldpen = (HPEN)::SelectObject(hdc, pen);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	::SelectObject(hdc, oldpen);
	::DeleteObject(pen);
}

wchar_t * wcscpy_lower(wchar_t * d, int d_len, const wchar_t *s)
{
	wchar_t * od = d;
	wchar_t * de = d + d_len - 1;
	while (d < de && *s)
	{
		*d++ = towlower(*s++);
	}
	*d = L'\0';
	return od;
}

int remove_char(std::wstring * str, wchar_t ch)
{
	int count = 0;
	for (size_t i = 0; i < str->length(); i++)
	{
		size_t index = str->length() - 1 - i;
		if (str->at(index) == ch)
		{
			str->erase(index, 1);
		}
	}
	return count;
}

int match_str_list(const wchar_t * list, const wchar_t * s)
{
	size_t l = wcslen(s);
	const wchar_t * p = list;
	while (*p)
	{
		if (_wcsnicmp(s, p, l) == 0 && is_space(*(p+l)))
		{
			return static_cast<int>(p - list);
		}
		p += l;
		while (*p && !is_space(*p)) p++;
		while (*p && is_space(*p)) p++;
	}
	return -1;
}

std::wstring quote_string(const wchar_t * str, wchar_t q)
{
	std::wstring s(1, q);
	s += str;
	s += q;
	return s;
}

void print_err(const wchar_t * fmt, ...)
{
	wchar_t buf[1024];
	va_list parg;
	va_start(parg, fmt);
	_vsnwprintf_s(buf, sizeof(buf)/sizeof(buf[0]), fmt, parg);
	::MessageBoxW(GetForegroundWindow(), buf, L"HandyRun", MB_OK | MB_ICONWARNING);
	va_end(parg);
}

// 得到last error的文字说明
void print_sys_err(const wchar_t * info)
{
	DWORD dwErr = GetLastError();
	HRESULT hr = HRESULT_FROM_WIN32(dwErr);
	print_com_error(info, hr);
}

void print_com_error(const wchar_t * info, HRESULT hr)
{
	const int EL = 1024;
	wchar_t szErr[EL];
	size_t len = wcslen(info);
	wcsncpy_s(szErr, info, EL);
	::FormatMessage(
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hr, 0,
		szErr + len, static_cast<DWORD>(EL - len), NULL);
	szErr[EL-1] = L'\0';
	print_err(szErr);
}

bool file_exists(const wchar_t *fn)
{
//	struct _stat st;
//	return (_wstat(fn, &st) == 0);
	HANDLE h = CreateFile(fn, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE) return false;
	CloseHandle(h);
	return true;
}

std::wstring get_tmp_file_name(const wchar_t * ext)
{
	wchar_t dir[MAX_PATH];
	wchar_t path[MAX_PATH];
	if (::GetTempPath(elem_of(dir), dir))
	{
		if (::GetTempFileName(dir, L"hrt", 0, path))
		{
			std::wstring str = path;
			if (ext) str += ext;
			return str;
		}
	}
	return hc::empty_str;
}

std::wstring abs_path(const wchar_t *p, bool file)
{
	// 1. 文件直接存在
	if (file_exists(p)) return p;
	// 2. 文件名含环境变量
	std::wstring t = expand_envvar(p);
	if (file_exists(t.c_str())) return t;
	// 3. 如果是文件的话,还要在%path%中搜索
	if (file)
	{
		t = path_resolve(p);
		if (file_exists(t.c_str())) return t;
	}

	return p;
}

std::wstring rela_path(const wchar_t *p)
{
	std::wstring best_match_var;
	size_t best_match_len = 0;
	wchar_t * envvar = GetEnvironmentStrings();
	for (wchar_t *v = envvar; *v; v += wcslen(v) + 1)
	{
		wchar_t *eq_sign = wcschr((*v == L'=' ? v+1 : v), L'=');
		if (eq_sign == NULL) continue;

		size_t len = wcslen(eq_sign + 1);
		if (len > best_match_len && _wcsnicmp(p, eq_sign+1, len) == 0)
		{
			best_match_len = len;
			best_match_var.assign(v, eq_sign - v);
		}
	}
	FreeEnvironmentStrings(envvar);
	if (best_match_len > 0)
	{
		std::wstring r = L"%";
		r += best_match_var + L"%";
		r += p + best_match_len;
		return r;
	}

	return p;
}

bool send_mouse_input(int x, int y, DWORD flags)
{
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	x = static_cast<int>(x * 65536.0 / cx);
	y = static_cast<int>(y * 65536.0 / cy);
	MOUSEINPUT mi = { x, y, 0, flags, 0, 0 };
	INPUT input = { INPUT_MOUSE };
	input.mi = mi;
	return SendInput(1, &input, sizeof(input)) > 0;
}

bool focus_window(HWND wnd)
{
	POINT pt;
	GetCursorPos(&pt);

	RECT rc;
	GetWindowRect(wnd, &rc);
	int x = rc.right - 1;
	int y = rc.bottom - 1;

	send_mouse_input(x, y, MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);
	send_mouse_input(x, y, MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
	send_mouse_input(x, y, MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);

	SetCursorPos(pt.x, pt.y);
	return true;
}

HICON get_file_icon(const wchar_t * fn)
{
	SHFILEINFO fi = {0};
	if (SHGetFileInfoW(fn, FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi), 
		SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
	{
		return fi.hIcon;
	}
	return NULL;
}

HICON CreateGrayscaleIcon(HICON hIcon)
{
    HICON       hGrayIcon = NULL;
    HDC         hMainDC = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
    BITMAP      bmp;
    HBITMAP     hOldBmp1 = NULL, hOldBmp2 = NULL;
    ICONINFO    csII;
    BOOL        bRetValue = FALSE;

    bRetValue = ::GetIconInfo(hIcon, &csII);
    if (bRetValue == FALSE) return NULL;

    hMainDC = ::GetDC(NULL);
    hMemDC1 = ::CreateCompatibleDC(hMainDC);
    hMemDC2 = ::CreateCompatibleDC(hMainDC);
    if (hMainDC == NULL || hMemDC1 == NULL || hMemDC2 == NULL) return NULL;
  
    if (::GetObject(csII.hbmColor, sizeof(BITMAP), &bmp))
    {
        DWORD   dwWidth = csII.xHotspot*2;
        DWORD   dwHeight = csII.yHotspot*2;

		ICONINFO csGrayII;
		csGrayII.hbmColor = ::CreateBitmap(dwWidth, dwHeight, bmp.bmPlanes,
                                           bmp.bmBitsPixel, NULL);
        if (csGrayII.hbmColor)
        {
            hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, csII.hbmColor);
            hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, csGrayII.hbmColor);

            //::BitBlt(hMemDC2, 0, 0, dwWidth, dwHeight, hMemDC1, 0, 0, 

            //         SRCCOPY);


            DWORD    dwLoopY = 0, dwLoopX = 0;
            COLORREF crPixel = 0;
            BYTE     byNewPixel = 0;

            for (dwLoopY = 0; dwLoopY < dwHeight; dwLoopY++)
            {
                for (dwLoopX = 0; dwLoopX < dwWidth; dwLoopX++)
                {
                    crPixel = ::GetPixel(hMemDC1, dwLoopX, dwLoopY);

                    byNewPixel = (BYTE)((GetRValue(crPixel) * 0.299) + 
                                        (GetGValue(crPixel) * 0.587) + 
                                        (GetBValue(crPixel) * 0.114));
                    if (crPixel) ::SetPixel(hMemDC2, dwLoopX, dwLoopY, 
                                            RGB(byNewPixel, byNewPixel, 
                                            byNewPixel));
                } // for

            } // for


            ::SelectObject(hMemDC1, hOldBmp1);
            ::SelectObject(hMemDC2, hOldBmp2);

            csGrayII.hbmMask = csII.hbmMask;

            csGrayII.fIcon = TRUE;
            hGrayIcon = ::CreateIconIndirect(&csGrayII);
			::DeleteObject(csGrayII.hbmColor);
        } // if

        //::DeleteObject(csGrayII.hbmMask);

    } // if


    ::DeleteObject(csII.hbmColor);
    ::DeleteObject(csII.hbmMask);
    ::DeleteDC(hMemDC1);
    ::DeleteDC(hMemDC2);
    ::ReleaseDC(NULL, hMainDC);

    return hGrayIcon;
} // End of CreateGrayscaleIcon



path_elem split_path(const wchar_t * path)
{
	path_elem pe = {};
	int i;
	for (i = 0; path[i] != '\0'; i++)
	{
		if (path[i] == L'\\') pe.name.s = i+1;
		if (path[i] == L'.')  pe.ext.s = i;
	}
	if (pe.ext.s < pe.name.s) pe.ext.s = i;

	pe.ext.n = i - pe.ext.s;
	pe.name.n = pe.ext.s - pe.name.s;
	pe.path.n = pe.name.s - pe.path.s;

	return pe;
}

size_t strcat_ex(wchar_t * buf, size_t buf_len, size_t pos, const wchar_t * src, size_t src_len)
{
	if (pos >= buf_len - 1) return pos;
	size_t vlen = buf_len - pos;
	wchar_t * vbuf = buf + pos;

	if (src_len == (size_t)-1)
	{
		src_len = wcslen(src);
	}

	size_t copy_len = src_len;
	if (copy_len > vlen-1) copy_len = vlen-1;

	wcsncpy_s(vbuf, vlen, src, copy_len);
	vbuf[copy_len] = L'\0';

	return pos + copy_len;
}

void show_err(const wchar_t *err, const wchar_t *result)
{
	wchar_t gap[] = {L'\n', L'\n', L'\0'};

	if (!err) return;

	if (!result)
	{
		result = hc::empty_str;
		gap[0] = L'\0';
	}

	HWND hwnd = GetForegroundWindow();
	if (!wnd_in_same_thread(hwnd)) hwnd = NULL;
	MessageBox(hwnd, tp::cz(L"%s%s%s", err, gap, result), 
		hc::handyrun_wnd_title, MB_OK | MB_ICONWARNING);
}

std::wstring hlp::expand_envvar(const wchar_t * p)
{
	std::wstring t;
	wchar_t buf[MAX_PATH];
	::ExpandEnvironmentStringsW(p, buf, elem_of(buf));
	t = buf;
	return t;
}

std::wstring path_resolve(const wchar_t * str)
{
	wchar_t buf[MAX_PATH];
	wcsncpy_s(buf, elem_of(buf), str, _TRUNCATE);
	::PathResolve(buf, NULL, PRF_TRYPROGRAMEXTENSIONS|PRF_VERIFYEXISTS);
	return buf;
}

bool is_dos_command(const std::wstring& cmd)
{
	if (!file_exists(cmd.c_str())) return false;
	DWORD_PTR exe_type = ::SHGetFileInfoW(cmd.c_str(), 0, NULL, 0, SHGFI_EXETYPE);
	return (exe_type != 0 && HIWORD(exe_type) == 0);
}

RECT get_dlg_item_rect(HWND wnd, int id)
{
	HWND ctrl_wnd = ::GetDlgItem(wnd, id);
	RECT rc;
	::GetWindowRect(ctrl_wnd, &rc);
	::ScreenToClient(wnd, (LPPOINT)&rc);
	::ScreenToClient(wnd, ((LPPOINT)&rc)+1);
	return rc;
}

const wchar_t * wcsistr(const wchar_t *s, const wchar_t *sub_s)
{
	struct inner
	{
		static wchar_t tolowerchar(wchar_t ch)
		{
			return static_cast<wchar_t>(tolower(ch));
		}
	};
	std::wstring s1(s);
	std::wstring s2(sub_s);
	std::transform(s1.begin(), s1.end(), s1.begin(), inner::tolowerchar);
	std::transform(s2.begin(), s2.end(), s2.begin(), inner::tolowerchar);
	const wchar_t * m = wcsstr(s1.c_str(), s2.c_str());
	if (m != NULL)
	{
		m = s + (m - s1.c_str());
	}
	return m;
}

bool hlp::track_mouse_event(DWORD flag, HWND wnd, DWORD hover_time)
{
	TRACKMOUSEEVENT tme = {
		sizeof(TRACKMOUSEEVENT), flag, wnd, hover_time
	};
	return (::TrackMouseEvent(&tme) == TRUE);
}

POINT hlp::get_cursor_pos(HWND wnd)
{
	POINT pt;
	GetCursorPos(&pt);
	::ScreenToClient(wnd, &pt);
	return pt;
}

bool wnd_in_same_process(HWND hwnd)
{
	DWORD process1, process2;
	GetWindowThreadProcessId(hwnd, &process1);
	process2 = GetCurrentProcessId();
	return process1 == process2;
}

bool wnd_in_same_thread(HWND hwnd)
{
	DWORD thread_id = ::GetWindowThreadProcessId(hwnd, NULL);
	return thread_id == ::GetCurrentThreadId();
}

bool is_key_down(int vk)
{
	return (::GetKeyState(vk) & 0x8000) != 0;
}


/*  wild_match带通配符的字符串比较-----------------------------------------
[in] wild   模式串,可以包含?*两种通配符,?匹配任意的单个字符,*匹配任意的字符串
[in] src    正常的字符串
返回值      true: 匹配   false: 不匹配

函数用一个模式串wild去和另一个串src匹配,返回匹配结果
如果wild和src长度都是0, 则返回true
*/
bool wild_match(const wchar_t *wild, const wchar_t *src)
{
	const wchar_t *cp = NULL;
	const wchar_t *mp = NULL;
	
	while ((*src) && (*wild != '*'))
	{
		if ((*wild != *src) && (*wild != '?'))
		{
			return false;
		}
		wild++;
		src++;
	}
	
	while (*src)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return true;
			}
			mp = wild;
			cp = src+1;
		} 
		else if ((*wild == *src) || (*wild == '?')) 
		{
			wild++;
			src++;
		} 
		else 
		{
			wild = mp;
			src = cp++;
		}
	}
		
	while (*wild == '*') 
	{
		wild++;
	}
	return !*wild;
}

HRESULT hlp::ShellExecuteByExplorer(const wchar_t* path, const wchar_t* param, const wchar_t* dir)
{
	HRESULT hr;
	static CComPtr<IShellDispatch2> psd;

	if (!psd)
	{
		CComPtr<IShellWindows> psw;
		hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&psw));
		if (FAILED(hr)) return hr;

		HWND hwnd;
		CComPtr<IDispatch> pdisp;
		VARIANT vEmpty = {}; // VT_EMPTY
		hr = psw->FindWindowSW(&vEmpty, &vEmpty, SWC_DESKTOP, (long*)&hwnd, SWFO_NEEDDISPATCH, &pdisp);
		if (FAILED(hr)) return hr;

		CComPtr<IShellBrowser> psb;
		hr = IUnknown_QueryService(pdisp, SID_STopLevelBrowser, IID_PPV_ARGS(&psb));
		if (FAILED(hr)) return hr;

		CComPtr<IShellView> psv;
		hr = psb->QueryActiveShellView(&psv);
		if (FAILED(hr)) return hr;

		CComPtr<IDispatch> pdispBackground;
		hr = psv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&pdispBackground));
		if (FAILED(hr)) return hr;

		CComPtr<IShellFolderViewDual> psfvd;
		hr = pdispBackground->QueryInterface(IID_PPV_ARGS(&psfvd));
		if (FAILED(hr)) return hr;

		CComPtr<IDispatch> pdisp2;
		hr = psfvd->get_Application(&pdisp2);
		if (FAILED(hr)) return hr;

		hr = pdisp2->QueryInterface(IID_PPV_ARGS(&psd));
		if (FAILED(hr)) return hr;
	}

	if (psd)
	{
		psd->ShellExecuteW(CComBSTR(path), CComVariant(param), CComVariant(dir), CComVariant(L"open"), CComVariant(SW_SHOW));
		return S_OK;
	}

	return E_FAIL;
}

}

