/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <inttypes.h>
#include <stdint.h>

#include "nptypes.h"
#include "npapi.h"
#include "npruntime.h"

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlcoll.h> // for CAtlList
#include <comutil.h> // for _bstr_t

#include <mshtmcid.h>

#include "..\WebBrowser.h"
#include "..\WebBrowserPool.h"

#include "..\NPObjectPtr.h"

class CPlugin
{
friend class ScriptablePluginObject;

public:
	static CWebBrowserPool* browserPool;

    CPlugin(NPP pNPInstance);
    ~CPlugin();

	// called by npruntime everytime a new CPlugin instance is initialized
    NPBool Init(NPWindow* pNPWindow);
	
	// called by npruntime everytime a new CPlugin instance is destroyed
    void Destroy();

	// called once after the plugin dll is loaded
	static void GlobalInit();

	// called before unloading of the plugin dll
	static void GlobalDestroy();

	bool IsInitialized(); // check if the plugin is initialized

	bool IsMainThread() {
		bool ret = GetCurrentThreadId() == m_ThreadId;
		ATLASSERT(ret == true);
		return ret;
	}

	CWebBrowser* GetWebBrowser() {
		return m_pWebBrowser;
	}

	void SetWebBrowser(CWebBrowser* web_browser) {
		if(m_pWebBrowser) {
			m_pWebBrowser = NULL;
			m_pWebBrowser->DestroyWindow();
		}
		m_pWebBrowser = web_browser;
		*m_pWebBrowser = *web_browser;
		if(m_pWebBrowser)
			static_cast<CWindow*>(m_pWebBrowser)->SetParent(m_hWnd);
	}

	// create a webbrowser control and load the url, if we don't have one.
	// if url starts with @, this is a memory address pointing to an 
	// existing CWebBrowser object in browser pool.
	bool InitBrowser(const char* url);

	// called by npruntime, only for Mac OS X
    int16_t HandleEvent(void* event);

	// called by npruntime to get a scriptable object
    NPObject *GetScriptableObject();

    // plugin methods exported via NPAPI
	bool GoBack() {
		if(!m_pWebBrowser)
			return false;
		return SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->GoBack());
	}

	bool GoForward() {
		if(!m_pWebBrowser)
			return false;
		return SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->GoForward());
	}

	bool Navigate(const char* url) {
		if(!m_pWebBrowser) {
			// If browser object is not created
			return InitBrowser(url); // try to initialize the browser
		}

		CComVariant vurl = url;
		CComVariant null;
		return SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->Navigate2(&vurl, &null, &null, &null, &null));
	}

	bool Refresh() {
		if(!m_pWebBrowser)
			return false;
		CComVariant level = REFRESH_NORMAL;
		return SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->Refresh2(&level));
	}

	bool Stop() {
		if(!m_pWebBrowser)
			return false;
		return SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->Stop());
	}

	bool SaveAs() {
		return DoOleCommand(OLECMDID_SAVEAS);
	}

	bool Print() {
		return DoOleCommand(OLECMDID_PRINT);
	}

	bool PrintPreview() {
		return DoOleCommand(OLECMDID_PRINTPREVIEW);
	}

	bool PrintSetup() {
		return DoOleCommand(OLECMDID_PAGESETUP);
	}

	bool Cut() {
		return DoOleCommand(OLECMDID_CUT);
	}

	bool Copy() {
		return DoOleCommand(OLECMDID_COPY);
	}

	bool Paste() {
		return DoOleCommand(OLECMDID_PASTE);
	}

	bool SelectAll() {
		return DoOleCommand(OLECMDID_SELECTALL);
	}

	bool Find() {
		// http://msdn.microsoft.com/en-us/library/aa769909(v=VS.85).aspx
		return DoHtmlCommand(IDM_FIND);
	}

	bool ViewSource() {
		// http://msdn.microsoft.com/en-us/library/aa769962(v=VS.85).aspx
		return DoHtmlCommand(IDM_VIEWSOURCE);
	}

	void Focus() {
		if(!m_pWebBrowser)
			return;
		// if(::GetFocus() != m_pWebBrowser->m_hWnd)
		//	m_pWebBrowser->SetFocus();

		// really set the focus to the html window.
		CComPtr<IDispatch> doc;
		if(SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->get_Document(&doc))) {
			CComQIPtr<IHTMLDocument2> htmlDoc = doc;
			if(htmlDoc) {
				CComPtr<IHTMLWindow2> window;
				if(SUCCEEDED(htmlDoc->get_parentWindow(&window))) {
					window->focus();
				}
			}
		}
	}

	// properties
	NPObject* GetCallbacks() {
		return m_pCallbackObject;
	}

	void SetCallbacks(NPObject* callbacks);

	bool GetCanBack() {
		return m_pWebBrowser ? m_pWebBrowser->GetCanBack() : false;
	}
	bool GetCanForward() {
		return m_pWebBrowser ? m_pWebBrowser->GetCanForward() : false;
	}

	bool GetCanRefresh() {
		return IsOleCommandEnabled(OLECMDID_REFRESH);
	}

	bool GetCanStop() {
		return IsOleCommandEnabled(OLECMDID_STOP);
	}

	bool GetCanCut() {
		return IsOleCommandEnabled(OLECMDID_CUT);
	}

	bool GetCanCopy() {
		return IsOleCommandEnabled(OLECMDID_COPY);
	}

	bool GetCanPaste() {
		return IsOleCommandEnabled(OLECMDID_PASTE);
	}

	long GetProgress() {
		return m_Progress;
	}

	long GetSecurity() {
		return m_SecureLockIcon;
	}

	char* GetUrl(); // returns a UTF-8 string newly-allocated with NPN_MemAlloc()
    char* GetTitle(); // returns a UTF-8 string newly-allocated with NPN_MemAlloc()
	char* GetStatusText(); // returns a UTF-8 string newly-allocated with NPN_MemAlloc()

	// other utility methods

	void UpdateLocation(BSTR url);
	void OnUpdateLocation();

	void UpdateTitle(BSTR text);
	void OnUpdateTitle();

	void UpdateProgress(long progress);
	void OnUpdateProgress(long progress);

	void UpdateStatusText(BSTR status);
	void OnUpdateStatusText();

	void UpdateSecureLockIcon(long icon_id);
	void OnUpdateSecureLockIcon(long icon_id);

	void NewTab(const char* url);
	void OnNewTab();

	void CloseTab();
	void OnCloseTab();

	bool FilterKeyPress(int keyCode, bool isAltDown, bool isCtrlDown, bool isShiftDown) {
		bool ret = false;
		if(m_pCallbackObject) {
			static NPIdentifier filterKeyPress_id = 0;
			if(filterKeyPress_id == 0)
				filterKeyPress_id = NPN_GetStringIdentifier("filterKeyPress");
			m_pCallbackObject.Invoke(m_pNPInstance, filterKeyPress_id, &ret, "bibbb", 
									 keyCode, isAltDown, isCtrlDown, isShiftDown);
		}
		return ret;
	}

	void SwitchBackToFirefox();
	void OnSwitchBackToFirefox();

	HWND GetHwnd() {
		return m_hWnd;
	}

	// get the real URL of the html page containing the plugin
	CString GetPageURL(void);

	// returns a UTF-8 string newly-allocated with NPN_MemAlloc()
	static char* Bstr2Utf8(BSTR bstr);

protected:

	bool DoOleCommand(OLECMDID id, OLECMDEXECOPT opt = OLECMDEXECOPT_DODEFAULT) {
		if(!m_pWebBrowser || !IsOleCommandEnabled(id))
			return false;
		return SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->ExecWB(id, opt, NULL, NULL));
	}

	bool DoHtmlCommand(int id, OLECMDEXECOPT opt = OLECMDEXECOPT_DODEFAULT) {
		if(!m_pWebBrowser)
			return false;
		CComPtr<IDispatch> disp;
		m_pWebBrowser->GetIWebBrowser2()->get_Document(&disp);
		CComQIPtr<IOleCommandTarget> cmdTarget = disp; // = m_WebBrowser;
		return SUCCEEDED(cmdTarget->Exec(&CGID_MSHTML, id, opt, NULL, NULL));
	}

	bool IsOleCommandEnabled(OLECMDID id) {
		if(!m_pWebBrowser)
			return false;
		OLECMDF flags;
		return (SUCCEEDED(m_pWebBrowser->GetIWebBrowser2()->QueryStatusWB(id, &flags)) && (flags & OLECMDF_ENABLED));
	}

	NPObject* GetDomDocument() {
		static NPIdentifier document_id = 0;
		if(document_id == 0)
			document_id = NPN_GetStringIdentifier("document");
		NPObject* documentObject = NULL;
		NPVariant documentVariant;
		if(NPN_GetProperty(m_pNPInstance, m_pWindowObject.p, document_id, &documentVariant)) {
			documentObject = NPVARIANT_TO_OBJECT(documentVariant);
		}
		return documentObject;
	}

	static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);

	// NPObject functions redirected from ScriptablePluginObject
    bool HasMethod(NPIdentifier name);
    bool HasProperty(NPIdentifier name);
    bool GetProperty(NPIdentifier name, NPVariant *result);
	bool SetProperty(NPIdentifier name, const NPVariant *value);
    bool Invoke(NPIdentifier name, const NPVariant *args,
                uint32_t argCount, NPVariant *result);
    bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                               NPVariant *result);
	static void RegisterIdentifiers();

private:
    NPP m_pNPInstance;
    HWND m_hWnd;
	NPWindow * m_Window;
    NPObject *m_pScriptableObject;
    CNPObjectPtr m_pWindowObject;
    CNPObjectPtr m_pCallbackObject;
    bool m_bInitialized;
	long m_Progress;
	long m_SecureLockIcon;
    CWebBrowser* m_pWebBrowser;
	WNDPROC m_lpOldProc; // old window proc of the Gecko plugin window
	DWORD m_ThreadId;

	CComBSTR m_StatusText;
};

#endif // __PLUGIN_H__
