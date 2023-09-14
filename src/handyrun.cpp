#include "stdafx.h"

#include <locale.h>
#include "resource.h"
#include "config.h"
#include "rundlg.h"
#include "hotkey_waker.h"
#include "msgdebuger.h"
#include "global_data.h"
#include "helper.h"
#include "defs.h"
#include "log.h"
#include "error.h"

#include "rapidxml/rapidxml.hpp"

#include <vector>

CAppModule _Module;

// 收到热键后，HandyRun判断前台窗口是否属于自己的进程。如果不是，则显示运行窗口；否则隐藏运行窗口。
void toggle_show_cmd()
{
	HWND hwnd = GetForegroundWindow();
	CRunDlg * dlg = CRunDlg::Instance();
	if (dlg->m_hWnd == NULL) dlg->Create(NULL);

	if (hlp::wnd_in_same_thread(hwnd))
	{
		dlg->PostMessageW(WM_SHOW_RUNDLG, FALSE);
	}
	else
	{
		dlg->PostMessageW(WM_SHOW_RUNDLG, TRUE);
	}
}

int Run(LPWSTR /*lpstrCmdLine*/ = NULL, int /*nCmdShow*/ = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	log_msg(L"initializing wake-up facility...");
	hotkey_waker * hw = hotkey_waker::instance();
	cfg::config * c = cfg::config::instance();
	hw->register_hotkey(c->go.active_key.c_str(), toggle_show_cmd);
	theLoop.AddMessageFilter(hw);
	
	if (gd::run_once)
	{
		toggle_show_cmd();
	}
	else if (c->gm.style == cfg::ws_edge)
	{
		CRunDlg * dlg = CRunDlg::Instance();
		if (!dlg->m_hWnd) dlg->Create(NULL);
	}

	log_msg(L"event loop");
	_Module.GetMessageLoop()->Run();
	theLoop.RemoveMessageFilter(hw);
	_Module.RemoveMessageLoop();
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR lpstrCmdLine, int nCmdShow)
{
	HANDLE hApp = NULL;

	log_msg(L"constructing path...");
	wchar_t buf[MAX_PATH];
	::GetModuleFileName(hInstance, buf, elem_of(buf));
	hlp::path_elem pe = hlp::split_path(buf);
	gd::hr_path.assign(buf, pe.path.n);
	gd::hr_ini = gd::hr_path + hc::fn_ini;
/*
	FILE* fp = NULL;
	_wfopen_s(&fp, L"E:\\greensoft\\notepad++\\localization\\basque.xml", L"rb");
	fseek(fp, 0, SEEK_END);
	long filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = new char[filelen + 1];
	fread(buffer, filelen, 1, fp);
	buffer[filelen] = 0;

	rapidxml::xml_document<> doc;
	doc.parse<0>(buffer);
*/
	log_msg(L"setting locale...");
	setlocale(LC_CTYPE, "");

	// TODO: 在没有配置文件时提示用户第一次运行

	cfg::config *c = cfg::config::instance();
	if (c->go.single_instance)
	{
		log_msg(L"creating mutex...");
		hApp = ::CreateMutex(NULL, FALSE, L"HandyRun_FFBE972IC");
		if (hApp && GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(hApp);
			return 1;
		}
	}

	// command line
	if (wcscmp(lpstrCmdLine, L"/once") == 0)
	{
		gd::run_once = true;
//		c->gm.style = cfg::ws_center;
	}

	Gdiplus::GdiplusStartupInput gpsi;
	ULONG_PTR gptoken;
	Gdiplus::GdiplusStartup(&gptoken, &gpsi, NULL);

	HRESULT hRes = ::OleInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	::DefWindowProc(NULL, 0, 0, 0L);

//	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls
	log_msg(L"initializing application...");
	hRes = _Module.Init(NULL, hInstance);
//	ATLASSERT(SUCCEEDED(hRes));

	int nRet = 0;
	try
	{
		nRet = Run(lpstrCmdLine, nCmdShow);
	}
	catch (handyrun_err &he)
	{
		hlp::show_err(he.what(), L"程序终止");
	}
	catch (const wchar_t * err)
	{
		hlp::show_err(err, L"程序终止");
	}
	catch (...)
	{
	}

	log_msg(L"terminating...");
	_Module.Term();
	::OleUninitialize();
	Gdiplus::GdiplusShutdown(gptoken);

//	if (gd::prev_active_wnd)
//		hlp::SetForegroundWindowEx(gd::prev_active_wnd);
	CloseHandle(hApp);
	return nRet;
}
