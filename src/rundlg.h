#pragma once
#include <list>
#include <string>
#include "resource.h"
#include "config.h"
#include "tipdlg.h"
#include "regiontipdlg.h"
#include "codesuggestpane.h"
#include "transparentedit.h"
#include "msgdebuger.h"
#include "prog.h"
#include "layout.h"
#include "droptargetimpl.h"
#include "custombkdlg.h"
#include "debug.h"

#define WM_SHOW_RUNDLG WM_USER + 0x426

using lyt::prog_pos;
using lyt::pp_null;

class CRunDlg
	: public CDialogImpl<CRunDlg>
	, public CRegionTipDlgImpl<CRunDlg>
	, public IDropTargetImpl<CRunDlg>
	, public CCustomBkDlgImpl<CRunDlg>
	, public CMsgDebuger<CRunDlg>
//	, public CMessageFilter
{
	friend class hover_state;
public:
	enum {IDD = IDD_RUNDLG};

	static CRunDlg * Instance();
	~CRunDlg();

private:
	class op_state
	{
	public:
		virtual void init(){};
		virtual void cleanup(){};
		virtual void handle_mouse(UINT msg, WPARAM wp, POINT pt) = 0;
	};
	class hover_state : public op_state
	{
	public:
		virtual void init();
		virtual void cleanup();
		virtual void handle_mouse(UINT msg, WPARAM wp, POINT pt);
	private:
		POINT m_last_mouse_pos;
	};
	class drag_prog_state : public op_state
	{
	public:
		virtual void init();
		virtual void cleanup();
		virtual void handle_mouse(UINT msg, WPARAM wp, POINT pt);
	private:
		POINT m_last_mouse_pos;
		prog_pos m_dragged_pp;
		CImageList m_il;
	};
	class drop_target_state : public op_state
	{
		// 有点特殊, 因为需要按windows定义的接口来实现
	public:
		virtual void init();
		virtual void cleanup();
		virtual void handle_mouse(UINT msg, WPARAM wp, POINT pt);
		
		DWORD drag_enter(IDataObject * data, POINT pt);
		DWORD drag_over(POINT pt);
		void  drag_leave();
		DWORD drop(IDataObject * data, POINT pt);

	private:
		static bool get_drop_files(IDataObject * data, std::list<std::wstring>* fl);
		static std::wstring get_drop_text(IDataObject *data);
		static int get_param_var_count(const std::wstring& param);
		std::wstring expand_drop_param(const std::wstring& param) const;
		POINT m_last_mouse_pos;
		CLIPFORMAT m_data_format;
		string_list_t m_cached_var;
		size_t m_var_count;
	};
	class drag_window_state : public op_state
	{
	public:
		virtual void init();
		virtual void cleanup();
		virtual void handle_mouse(UINT msg, WPARAM wp, POINT pt);
	private:
		POINT m_pressed_pos;
	};
	class run_state : public op_state
	{
	public:
		virtual void init();
		virtual void cleanup();
		virtual void handle_mouse(UINT msg, WPARAM wp, POINT pt);
	private:
		prog_pos m_pressed_pp;
		POINT m_pressed_pos;
	};

	CRunDlg();
	void EndRequest();
	enum ops{os_hover, os_run, os_drag_wnd, os_drag_prog, os_drop_target};
	void change_op_state(ops os);
	void refresh_layout();
	void show_window(bool show, bool force_focus = false);
	void place_window_on_edge(bool show);
	void ui_exit();
	void AdjustWndSize();
	virtual LRESULT OnTTGetDisp(CToolTipCtrl & tip_ctrl, LPNMHDR pnmh, BOOL &);
	bool DoExecute(const command * cmd);
	void reload_profile(void (*pf_modify_op)());
	void enable_type_command(bool enable);
	void OnPressEnter();
	void OnClickProg(const prog * pi);
	bool can_fold() const;
	void on_setting_change();

	lyt::layout * m_lyt;
	op_state * m_os;
	CEdit m_edit;
	CTipDlg m_tip;
	POINT m_menu_point;
	bool m_current_show_state;
	hover_state m_os_hover;
	run_state m_os_run;
	drag_window_state m_os_drag_wnd;
	drag_prog_state m_os_drag_prog;
	drop_target_state m_os_drop_target;
	prog_pos m_curr_prog_pos;
	bool m_last_cmd_forced;
	prog * m_prog_waiting_param;        // 正等待足够参数运行的prog
	HBITMAP m_background;
	CComPtr<IShellDispatch2> m_pSD;
private: // 消息映射

	BEGIN_MSG_MAP(CRunDlg)
		AtlTrace(L"msg: %s\n", dbg::get_msg_name(uMsg));
		CHAIN_MSG_MAP(CMsgDebuger<CRunDlg>)
COMMAND_ID_HANDLER(ID_INSERT_GROUP, OnInsertGroup)
COMMAND_ID_HANDLER(ID_EDIT_GROUP, OnEditGroup)
COMMAND_ID_HANDLER(ID_DEL_GROUP, OnDelGroup)
CHAIN_MSG_MAP(CRegionTipDlgImpl<CRunDlg>)
CHAIN_MSG_MAP(IDropTargetImpl<CRunDlg>)
CHAIN_MSG_MAP(CCustomBkDlgImpl<CRunDlg>)

		COMMAND_ID_HANDLER(ID_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_EDIT_PROFILE, OnEditProfile)
		COMMAND_ID_HANDLER(ID_CHANGE_PROFILE, OnChangeProfile)
		COMMAND_ID_HANDLER(ID_INSERT_PROG, OnInsertProg)
		COMMAND_ID_HANDLER(ID_EDIT_PROG, OnEditProg)
		COMMAND_ID_HANDLER(ID_DEL_PROG, OnDelProg)
		COMMAND_ID_HANDLER(ID_IMPORT_PROG, OnImportProg)
		COMMAND_ID_HANDLER(ID_CHLYT_GROUP, OnChlytGroup)
		COMMAND_ID_HANDLER(ID_CHLYT_PLAIN, OnChlytPlain)
		COMMAND_ID_HANDLER(ID_CHLYT_SIMPLE, OnChlytSimple)
		COMMAND_ID_HANDLER(ID_CHSTYLE_CENTER, OnChstyleCenter)
		COMMAND_ID_HANDLER(ID_CHSTYLE_EDGE, OnChstyleEdge)
		COMMAND_ID_HANDLER(ID_EXIT, OnExit)
		COMMAND_HANDLER(IDC_PROG_EDIT, EN_CHANGE, OnEnChangeProgEdit)

		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SHOW_RUNDLG, OnShow)
		MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivateApp)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseAction)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseAction)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseAction)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseAction)
		MESSAGE_HANDLER(WM_NCMOUSEMOVE, OnNcMouseAction)
		MESSAGE_HANDLER(WM_NCMOUSELEAVE, OnNcMouseAction)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_ENDSESSION, OnEndSession)
//		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDOK, OnOK)
	END_MSG_MAP()

	LRESULT OnEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditProfile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChangeProfile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInsertProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnImportProg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChlytGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChlytPlain(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChlytSimple(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChstyleCenter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChstyleEdge(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivateApp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseAction(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcMouseAction(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEnChangeProgEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInsertGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	HRESULT OnDragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT OnDragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT OnDragLeave();
	HRESULT OnDrop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
};
