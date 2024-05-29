#pragma once

#include <string>

template <class T>
class ATL_NO_VTABLE IDropTargetImpl :
	public IDropTarget,
	public CMessageMap
{
public:
	BEGIN_MSG_MAP(IDropTargetImpl)
//		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,OnDestory)
	END_MSG_MAP()

	HRESULT InitDropTarget()
	{
		T* pT = static_cast<T*>(this);
		IDropTarget *pDropTarget;
		QueryInterface(IID_IDropTarget,(void **)&pDropTarget);
		HRESULT hr = CoLockObjectExternal(pDropTarget, TRUE, FALSE);
		if (SUCCEEDED(hr)) 
		{
			hr = RegisterDragDrop(pT->m_hWnd, pDropTarget);
		}
		return hr;
	}
	LRESULT OnDestory(UINT /*uMsg*/, WPARAM , LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		RevokeDragDrop(pT->m_hWnd);
		IDropTarget *pDropTarget;
		QueryInterface(IID_IDropTarget,(void **)&pDropTarget);
		HRESULT hr = CoLockObjectExternal(pDropTarget, FALSE, TRUE);
		bHandled=FALSE;
		return SUCCEEDED(hr)? S_OK : S_FALSE;
	}

	//IUnknown implementation
	HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject)
	{
		if(iid == IID_IDropTarget || iid == IID_IUnknown)
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}
		else
		{
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}
	ULONG __stdcall AddRef (void){return 1;}
	ULONG __stdcall Release (void){return 1;}

	HRESULT OnDragEnter(IDataObject * , DWORD , POINTL , DWORD *)
	{
		return S_OK;
	}
	HRESULT OnDragOver (DWORD , POINTL , DWORD * )
	{
		return S_OK;
	}
	HRESULT OnDragLeave()
	{
		return S_OK;
	}
	HRESULT OnDrop (IDataObject * , DWORD , POINTL , DWORD * )
	{
		return S_OK;
	}
	// IDropTarget implementation
	HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnDragEnter(pDataObject, grfKeyState, pt,  pdwEffect);
	}
	HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnDragOver (grfKeyState, pt, pdwEffect);
	}
	HRESULT __stdcall DragLeave (void)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnDragLeave ();
	}
	HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnDrop (pDataObject,grfKeyState, pt, pdwEffect);
	}
};
