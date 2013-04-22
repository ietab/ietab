//
// WebBrowser.cpp: creates IWebBrowser2 control
//
// Copyright (C) 2012 - 2013 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
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

CWebBrowser::CWebBrowser(CPlugin* plugin):
	m_Plugin(plugin),
	m_pIWebBrowser2(NULL),
	m_hInnerWnd(NULL),
	m_CanBack(false),
	m_CanForward(false),
	m_OldInnerWndProc(NULL),
	m_Destroyed(false) {
}

CWebBrowser::~CWebBrowser() {
}

// static
CWebBrowser* CWebBrowser::FromHwnd(HWND hwnd) {
	CWebBrowser* pWebBrowser;
	do {
		pWebBrowser = reinterpret_cast<CWebBrowser*>(GetProp(hwnd, reinterpret_cast<LPCTSTR>(winPropAtom)));
		hwnd = ::GetParent(hwnd);
	}while(pWebBrowser == NULL && hwnd != NULL);
	return pWebBrowser;
}

// The message loop of Mozilla does not handle accelertor keys.
// IOleInplaceActivateObject requires MSG be filtered by its TranslateAccellerator() method.
// So we install a hook to do the dirty hack.
// Mozilla message loop is here:
// http://mxr.mozilla.org/mozilla-central/source/widget/src/windows/nsAppShell.cpp
// bool nsAppShell::ProcessNextNativeEvent(bool mayWait)
// It does PeekMessage, TranslateMessage, and then pass the result directly
// to DispatchMessage.
bool CWebBrowser::PreTranslateMessage(MSG* msg) {
	bool ret = false;
	// here we only handle keyboard messages
	if((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) ||
		(msg->message >= WM_MOUSEFIRST || msg->message <= WM_MOUSELAST)) {
		// get the browser object from HWND
		CWebBrowser* pWebBrowser = FromHwnd(msg->hwnd);
		if(pWebBrowser != NULL) {
			bool needTranslateAccelerator = true;
			// Let the browser filter the key event first
			if(pWebBrowser->GetPlugin()) {
				if(msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN) {
					// we only pass the key to plugin if the browser does not want it.
					bool isAltDown = GetKeyState(VK_MENU) & 0x8000 ? true : false;
					bool isCtrlDown = GetKeyState(VK_CONTROL) & 0x8000 ? true : false;
					bool isShiftDown = GetKeyState(VK_SHIFT) & 0x8000 ? true : false;
					if(pWebBrowser->GetPlugin()->FilterKeyPress(static_cast<int>(msg->wParam), isAltDown, isCtrlDown, isShiftDown)) {
						needTranslateAccelerator = false; // the browser wants it!
					}
				}
				if(needTranslateAccelerator) {
					if(pWebBrowser->m_pInPlaceActiveObject->TranslateAccelerator(msg) == S_OK) {
						ret = true;
					}
				}
			}
		}
	}
	return ret;
}

LRESULT CWebBrowser::OnCreate(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled) {

	// See ATL source code in atlhost.h
	// static LRESULT CALLBACK AtlAxWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	// ATL creates a new CAxHostWindow object when handling WM_CREATE message.
	// We intercept WM_CREATE, and create our CWebBrowserHost object instead.

	// This is to make sure drag drop works
	::OleInitialize(NULL);

	CREATESTRUCT* lpCreate = (CREATESTRUCT*)lParam;
	int nCreateSize = 0;
	if (lpCreate && lpCreate->lpCreateParams)
		nCreateSize = *((WORD*)lpCreate->lpCreateParams);
	HGLOBAL h = GlobalAlloc(GHND, nCreateSize);
	CComPtr<IStream> spStream;
	if (h && nCreateSize) {
		BYTE* pBytes = (BYTE*) GlobalLock(h);
		BYTE* pSource = ((BYTE*)(lpCreate->lpCreateParams)) + sizeof(WORD); 
		//Align to DWORD
		//pSource += (((~((DWORD)pSource)) + 1) & 3);
		memcpy(pBytes, pSource, nCreateSize);
		GlobalUnlock(h);
		CreateStreamOnHGlobal(h, TRUE, &spStream);
	}

	IAxWinHostWindow* pAxWindow = NULL;
	CComPtr<IUnknown> spUnk;
	LPCTSTR lpstrName = _T("about:blank");
	HRESULT hRet = CWebBrowserHost::AxCreateControlLicEx(T2COLE(lpstrName), m_hWnd, spStream, &spUnk, NULL, IID_NULL, NULL, NULL);

	if(FAILED(hRet))
		return -1;	// abort window creation
	hRet = spUnk->QueryInterface(__uuidof(IAxWinHostWindow), (void**)&pAxWindow);
	if(FAILED(hRet))
		return -1;	// abort window creation

	SetWindowLongPtr(GWLP_USERDATA, (DWORD_PTR)pAxWindow);
	// continue with DefWindowProc
	LRESULT ret = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	bHandled = TRUE;

	// Now, the Web Browser Active X control is created.
	// Let's do what we want.

	// Set up event sink
	IUnknown* pUnk = NULL;
	if(SUCCEEDED(QueryControl<IUnknown>(&pUnk))) {

		AtlGetObjectSourceInterface(pUnk, &m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
		DispEventAdvise(pUnk, &DIID_DWebBrowserEvents2);

		pUnk->QueryInterface(IID_IWebBrowser2, (void**)&m_pIWebBrowser2);
		pUnk->Release();

		m_pInPlaceActiveObject = m_pIWebBrowser2; // store the IOleInPlaceActiveObject iface for future use
		m_pIWebBrowser2->put_RegisterAsBrowser(VARIANT_TRUE);
		m_pIWebBrowser2->put_RegisterAsDropTarget(VARIANT_TRUE);
	}

	++browserCount;
	if(browserCount == 1) {
		winPropAtom = GlobalAddAtom(L"IETab::WebBrowser");
	}
	return ret;
}

LRESULT CWebBrowser::OnDestroy(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled) {
	if(m_Destroyed == false) {
		// It seems that ATL incorrectly calls OnDestroy twice for one window.
		// So we need to guard OnDestroy with a flag. Otherwise we will uninitilaize things 
		// twice and get cryptic and terrible crashes.

		// If we're in the pool, remove ourself from it
		POSITION pos = CPlugin::browserPool->Find(this);
		if(pos)
			CPlugin::browserPool->RemoveAt(pos);

		m_pIWebBrowser2.Release();
		m_pInPlaceActiveObject.Release();

		if(m_hInnerWnd != NULL) { // if we subclassed the inner most Internet Explorer Server window
			// subclass it back
			::SetWindowLongPtr(m_hInnerWnd, GWL_WNDPROC, (LONG_PTR)m_OldInnerWndProc);
			ATLTRACE("window removed!!\n");
			RemoveProp(m_hInnerWnd, reinterpret_cast<LPCTSTR>(winPropAtom));
			m_hInnerWnd = NULL;
		}

		--browserCount;
		ATLASSERT(browserCount >=0); // this value should never < 0. otherwise it's a bug.

		if(0 == browserCount) {
			GlobalDeleteAtom(winPropAtom);
			winPropAtom = NULL;
		}
		m_Destroyed = true;
	}
	bHandled = TRUE;
	return 0;
}

// NewWindow3 is only available after Window xp SP2
void CWebBrowser::OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, long flags, BSTR bstrUrlContext, BSTR bstrUrl) {
	// ATLTRACE("NewWindow3\n");
	CWebBrowser* newWebBrowser = CPlugin::browserPool->AddNew(CString(bstrUrl));
	// FIXME: what will happen if the top level parent is destroyed before we have a new tab?
	if(newWebBrowser) {
		newWebBrowser->GetIWebBrowser2()->get_Application(ppDisp);
		if(m_Plugin) {
			char* url = CPlugin::Bstr2Utf8(bstrUrl);
			m_Plugin->NewTab(url);
			NPN_MemFree(url);
		}
		// FIXME: if we don't have a plugin here, the web browser will never be shown
		// How to solve this in the future releases?
	}
}

/*
void CWebBrowser::OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel) {
}
*/

void CWebBrowser::OnBeforeNavigate2(IDispatch *pDisp, VARIANT *url, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel) {
	// ATLTRACE("BeforeNavigage\n");
	CComBSTR location = url->bstrVal;
	if(location == "ietab:switchback") { // switch back to firefox
		*Cancel = true;
		if(m_Plugin) {
			m_Plugin->SwitchBackToFirefox();
		}
	}
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
		m_OldInnerWndProc = (WNDPROC)::SetWindowLongPtr(m_hInnerWnd, GWL_WNDPROC, (LONG_PTR)InnerWndProc);
		SetProp(m_hInnerWnd, reinterpret_cast<LPCTSTR>(winPropAtom), reinterpret_cast<HANDLE>(this));
	}
	// ATLTRACE("NavigateComplete\n");
	if(m_Plugin) {
		m_Plugin->UpdateLocation(URL->bstrVal);
	}
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
	if(m_Plugin) {
		m_Plugin->UpdateStatusText(Text);
	}
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
LRESULT CWebBrowser::HandleInnerWndProc(UINT message, WPARAM wparam, LPARAM lparam) {

	// This is called from the WndProc of the innermost Internet_Explorer_Server window
	// of IE web control. It's not exposed by the IWebBrowser interface, but we need it.
	// So we get its HWND with a dirty way and did window subclassing to intercept its events.
	//
	// Firefox > 3 introduced the so-called OOPP and uses quite a lot of dirty hacks to
	// make cross-process window hierachy possible. However it's very problematic and
	// instead of make the design cleaner, the Firefox team only added dirty hacks for famous
	// plugins to make them work. So that's why only flash, adobe pdf, and several famous ones work.
	// The problem is, each process has its own message queue and input focus. Windows from
	// different process are independent from each other. To make OOPP work, dirty hacks are needed.
	// 
	// From source code of Mozilla, we now know that it relies on WM_MOUSEACTIVATE and WM_KILLFOCUS
	// messages sent by Windows to the plugin window (window class name: GeckoPluginWindow).
	// When the plugin window running in the child process receives WM_MOUSEATVIATE, it by default
	// send a notification via IPC to the parent Firefox window to tell it that the plugin get the focus.
	// When it receives WM_KILLFOCUS, it again sends an IPC message to tell parent process that
	// the plugin losses the focus and the main firefox window should grab the focus.
	// However, this only works if the plugin window does not have any child windows.
	// If its child window gets focused or activated by mouse, the messages are not passed
	// to the plugin window directly and of course the firefox mechanism to handle focus becomes broken.
	// So the simple solution, an even dirtier hack is, send fake WM_MOUSEACTIVATE & WM_KILLFOCUS
	// messages to the plugin window running in the child process, so it can do proper IPC with
	// the main process.

	// References: OOPP IPC-related Mozilla Firefox source code:
	// http://dxr.mozilla.org/mozilla-central/dom/plugins/ipc/PluginInstanceChild.cpp.html#l1461
	// http://dxr.mozilla.org/mozilla-central/--GENERATED--/ipc/ipdl/PPluginInstanceChild.cpp.html#l1456
	// http://dxr.mozilla.org/mozilla-central/dom/plugins/ipc/PluginInstanceParent.cpp.html#l38
	// http://dxr.mozilla.org/mozilla-central/--GENERATED--/ipc/ipdl/PPluginInstanceParent.cpp.html#l2494

	if(m_Plugin) {
		switch(message) {
			case WM_MOUSEACTIVATE: {
				// ATLTRACE("child WM_MOUSEACTIVATE\n");
				::SendMessage(m_Plugin->GetHwnd(), WM_MOUSEACTIVATE, wparam, lparam);
				break;
			}
			/* I think firefox does not utilize this
			case WM_SETFOCUS: {
				// ATLTRACE("child WM_SETFOCUS\n");
				::SendMessage(m_Plugin->GetHwnd(), WM_SETFOCUS, 0, 0);
				break;
			}
			*/
			case WM_KILLFOCUS: {
				// ATLTRACE("child WM_KILLFOCUS\n");
				::PostMessage(m_Plugin->GetHwnd(), WM_KILLFOCUS, 0, 0);
				break;
			}
		}
	}
	return CallWindowProc(m_OldInnerWndProc, m_hInnerWnd, message, wparam, lparam);
}

// WindowProc for innermost Internet_Explorer_Server window.
LRESULT CWebBrowser::InnerWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	CWebBrowser* pWebBrowser = reinterpret_cast<CWebBrowser*>(GetProp(hwnd, reinterpret_cast<LPCTSTR>(winPropAtom)));
	if(pWebBrowser)
		return pWebBrowser->HandleInnerWndProc(message, wparam, lparam);
	return 0; // This is not possible
}
