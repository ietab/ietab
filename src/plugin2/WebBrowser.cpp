//
// WebBrowser.cpp: creates IWebBrowser2 control
//
// Copyright (C) 2012 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http:www.gnu.org/licenses/>.
//

#include "WebBrowser.h"
#include <windowsx.h>
#include "npruntime/plugin.h"
#include "webbrowser.h"

int CWebBrowser::browserCount = 0;
ATOM CWebBrowser::winPropAtom = 0;
HHOOK CWebBrowser::getMsgHook = NULL;

CWebBrowser::CWebBrowser(CPlugin* plugin):
	m_Plugin(plugin),
	m_hInnerWnd(NULL),
	m_CanBack(false),
	m_CanForward(false),
	m_OldInnerWndProc(NULL),
	m_ClientSite(NULL) {
}

CWebBrowser::~CWebBrowser() {
}

// The message loop of Mozilla does not handle accelertor keys.
// IOleInplaceActivateObject requires MSG be filtered by its TranslateAccellerator() method.
// So we install a hook to do the dirty hack.
// Mozilla message loop is here:
// http://mxr.mozilla.org/mozilla-central/source/widget/src/windows/nsAppShell.cpp
// bool nsAppShell::ProcessNextNativeEvent(bool mayWait)
// It does PeekMessage, TranslateMessage, and then pass the result directly
// to DispatchMessage.
// Just before PeekMessage returns, our hook procedure is called.
LRESULT CALLBACK CWebBrowser::GetMsgHookProc(int code, WPARAM wParam, LPARAM lParam) {
	LRESULT ret = CallNextHookEx(getMsgHook, code, wParam, lParam);
	if(code == HC_ACTION || code >= 0) {
		if(wParam == PM_REMOVE) { // GetMessage() is called
			MSG* msg = reinterpret_cast<MSG*>(lParam);
			// here we only handle keyboard messages
			if(msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) {
				// get the browser object from HWND
				CWebBrowser* pWebBrowser = reinterpret_cast<CWebBrowser*>(GetProp(msg->hwnd, reinterpret_cast<LPCTSTR>(winPropAtom)));
				if(pWebBrowser != NULL) {

					bool needTranslateAccelerator = true;
					// Let the browser filter the key event first
					if(pWebBrowser->GetPlugin()) {

						if(msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN) {
							// we only pass the key to plugin if the browser does not want it.
							bool isAltDown = GetKeyState(VK_MENU) & 0x8000 ? true : false;
							bool isCtrlDown = GetKeyState(VK_CONTROL) & 0x8000 ? true : false;
							bool isShiftDown = GetKeyState(VK_SHIFT) & 0x8000 ? true : false;
							if(pWebBrowser->GetPlugin()->filterKeyPress(msg->wParam, isAltDown, isCtrlDown, isShiftDown)) {
								needTranslateAccelerator = false; // the browser wants it!
								// forward the message to parent window.
								HWND toplevel = ::GetParent(pWebBrowser->GetPlugin()->GetHwnd());
								::PostMessage(toplevel, msg->message, msg->wParam, msg->lParam);
								msg->message = WM_NULL; // eat the message
								// FIXME: can we just set msg->hwnd to toplevel? will this cause any problems?
							}
						}

						if(needTranslateAccelerator) {
							if(pWebBrowser->m_pInPlaceActiveObject->TranslateAccelerator(msg) == S_OK ) {
								// NOTE: What a dirty hack!
								// According to MSDN, translated MSGs should not be passed again to
								// TranslateMessage().
								// Unfortunately, message loop of Firefox does not consider accelerators
								// and call TranslateMessage() and DispatchMessage() unconditionally.
								// So, here we call DispatchMessage() ourself instead of letting Firefox
								// do it, and eat the message by setting it to NULL. This way we can
								// bypass TranslateMessage() called by Firefox.
								// I, however, am not sure if this is correct.
								// MSDN did not tell us whether we should pass the message to other hooks
								// before or after we change its content.
								DispatchMessage(msg);
								ATLTRACE("TRANSLATED!!\n");
								msg->message = WM_NULL; // eat the message
							}
						}
					}
				}
			}
		}
	}
	return ret; // we don't touch the return value of other hooks
}

LRESULT CWebBrowser::OnCreate(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled) {
	// We must call DefWindowProc before we can attach to the control.
	LRESULT ret = DefWindowProc( uMsg, wParam,lParam);

	IUnknown* pUnk = NULL;
	if(SUCCEEDED(QueryControl<IUnknown>(&pUnk))) {

		AtlGetObjectSourceInterface(pUnk, &m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
		DispEventAdvise(pUnk, &DIID_DWebBrowserEvents2);

		pUnk->QueryInterface(IID_IWebBrowser2, (void**)(CComPtr<IWebBrowser2>*)this);
		pUnk->Release();

		m_pInPlaceActiveObject = *this; // store the IOleInPlaceActiveObject iface for future use
		(*this)->put_RegisterAsBrowser(VARIANT_TRUE);
		(*this)->put_RegisterAsDropTarget(VARIANT_TRUE);

		// Setup a customized client site
		CComObject<CCustomClientSite>::CreateInstance(&m_ClientSite); // What is we directly use new operator?
		m_ClientSite->AddRef(); // This seems to be needed?
		// m_ClientSite = new CComObject<CCustomClientSite>();

		CComQIPtr<IObjectWithSite> pObjectWithSite;
		if(SUCCEEDED(QueryHost(&pObjectWithSite))) {
			//pObjectWithSite->SetSite(m_ClientSite->GetUnknown());
		}

		CComQIPtr<IOleObject> pOleObject(pUnk);
		if(pOleObject) {
			CComQIPtr<IOleClientSite> pOleClientSite(m_ClientSite->GetUnknown());
			//pOleObject->SetClientSite(pOleClientSite);
		}

	}
	// ShowScrollBar(SB_BOTH, FALSE);

	++browserCount;
	if(getMsgHook == NULL) {
		// install the windows hook if needed
		HINSTANCE inst = static_cast<CComModule*>(_pAtlModule)->GetModuleInstance();
		getMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgHookProc, (HMODULE)inst, 0);

		winPropAtom = GlobalAddAtom(L"IETab::WebBrowser");
	}

	SetProp(m_hWnd, reinterpret_cast<LPCTSTR>(winPropAtom), reinterpret_cast<HANDLE>(this));
	return ret;
}

LRESULT CWebBrowser::OnDestroy(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled) {

	m_pInPlaceActiveObject.Release();

	CComPtr<IUnknown> pUnk;
	if(SUCCEEDED(QueryControl(&pUnk)))
	{
		// disconnect the sink
		DispEventUnadvise(pUnk, &DIID_DWebBrowserEvents2);

		CComQIPtr<IOleObject> pOleObject(pUnk);
		if(pOleObject)
			pOleObject->SetClientSite(NULL);
	}

	CComQIPtr<IObjectWithSite> pObjectWithSite;
	if(SUCCEEDED(QueryHost(&pObjectWithSite))) {
		pObjectWithSite->SetSite(NULL);
	}

	if(m_ClientSite) {
		// delete m_ClientSite;
		m_ClientSite->Release();
		m_ClientSite = NULL;
	}

	if(m_hInnerWnd != NULL) {

		// subclass it back
		// ::SetWindowLongPtr(m_hInnerWnd, GWL_WNDPROC, (LONG_PTR)m_OldInnerWndProc);
		m_hInnerWnd = NULL;
		ATLTRACE("window removed!!\n");
		RemoveProp(m_hInnerWnd, reinterpret_cast<LPCTSTR>(winPropAtom));
	}
	RemoveProp(m_hWnd, reinterpret_cast<LPCTSTR>(winPropAtom));

	--browserCount;
	// uninstall the hook if no one needs it now
	if(getMsgHook && 0 == browserCount) {
		UnhookWindowsHookEx(getMsgHook);
		getMsgHook = NULL;

		GlobalDeleteAtom(winPropAtom);
		winPropAtom = NULL;
	}
	bHandled = TRUE;
	return 0;
}

void CWebBrowser::OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel) {
	// ATLTRACE("NewWindow2\n");
	CWebBrowser* newWebBrowser = new CWebBrowser(NULL);
	RECT rc = {0};

	// FIXME: what will happen if the top level parent is destroyed before we have a new tab?
	HWND hwnd = newWebBrowser->Create(GetTopLevelParent(), rc);
	if(hwnd) {
		// (*newWebBrowser)->QueryInterface(IID_IDispatch, (void**)ppDisp);
		(*newWebBrowser)->get_Application(ppDisp);
		CPlugin::browserPool.push_back(newWebBrowser);
		if(m_Plugin) {
			m_Plugin->NewTab(newWebBrowser);
		}
	}
	else
		delete newWebBrowser;
}

void CWebBrowser::OnBeforeNavigate2(IDispatch *pDisp, VARIANT *url, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel) {
	// ATLTRACE("BeforeNavigage\n");
}


void CWebBrowser::OnNavigateComplete2(IDispatch* pDisp,  VARIANT* URL) {
	// NOTE: dirty hack!! We create the control by loading "about:blank".
	// So soon after the web page "about:blank" is loaded, it's child window
	// which really contains the Internet_Explorer_Server window is created.
	// The first NavigateComplete2 event we got is for "about:blank".
	// So we try to get the inner most child window here
	if(m_hInnerWnd == NULL) {
		m_hInnerWnd = m_hWnd;
		for(;;) {
			HWND child = ::GetWindow(m_hInnerWnd, GW_CHILD);
			if(child == NULL)
				break;
			m_hInnerWnd = child;
		}
		// subclass the window
		// m_OldInnerWndProc = (WNDPROC)::SetWindowLongPtr(m_hInnerWnd, GWL_WNDPROC, (LONG_PTR)InnerWndProc);
		SetProp(m_hInnerWnd, reinterpret_cast<LPCTSTR>(winPropAtom), reinterpret_cast<HANDLE>(this));
	}
	// ATLTRACE("NavigateComplete\n");
	if(m_Plugin) {
		m_Plugin->UpdateLocation(URL->bstrVal);
	}
}

void CWebBrowser::OnDocumentComplete(IDispatch *pDisp, VARIANT *URL) {
	// ATLTRACE("DocumentComplete\n");
}


void CWebBrowser::OnProgressChange(long Progress, long ProgressMax) {
	long percent;
	if(ProgressMax > 0) {
		percent = Progress * 100 / ProgressMax;
	}
	else
		percent = 0;
	if(m_Plugin)
		m_Plugin->UpdateProgress(percent);
}

void CWebBrowser::OnSetSecureLockIcon(long SecureLockIcon) {
	if(m_Plugin)
		m_Plugin->UpdateSecureLockIcon(SecureLockIcon);
}

void CWebBrowser::OnStatusTextChange(BSTR Text) {
	if(m_Plugin)
		m_Plugin->UpdateStatusText(Text);
}

void CWebBrowser::OnTitleChange(BSTR Text) {
	if(m_Plugin)
		m_Plugin->UpdateTitle(Text);
}

void CWebBrowser::OnWindowClosing(VARIANT_BOOL IsChildWindow, VARIANT_BOOL *Cancel) {
	// Let's cancel it, so IE won't destroy the web browser control.
	// Then, we call Firefox to close the tab for us.
	*Cancel = VARIANT_TRUE;
	if(m_Plugin)
		m_Plugin->CloseTab();
	else {
		// FIXME: handle the case when window is closed while it's still in the browserPool.
	}
}

void CWebBrowser::OnCommandStateChange(long Command, VARIANT_BOOL Enable) {
	switch(Command) {
	case CSC_NAVIGATEFORWARD:
		m_CanForward = (Enable == VARIANT_TRUE);
		break;
	case CSC_NAVIGATEBACK:
		m_CanBack = (Enable == VARIANT_TRUE);
		break;
	}
}

void CWebBrowser::OnFinalMessage(HWND hwnd) {
	/* cleanup when the window is destroyed */
}

// WindowProc for innermost Internet_Explorer_Server window.
LRESULT CWebBrowser::InnerWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	CWebBrowser* pWebBrowser = reinterpret_cast<CWebBrowser*>(GetProp(hwnd, reinterpret_cast<LPCTSTR>(winPropAtom)));
	return CallWindowProc(pWebBrowser->m_OldInnerWndProc, hwnd, message, wparam, lparam);
}
