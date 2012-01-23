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
 * 2011-12-25 modifined by Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 * for use in IE Tab plugin.
 *
 * ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////
//
// CPlugin class implementation
//

#include <stdio.h>
#include "plugin.h"
#include <windows.h>
#include <windowsx.h>
#include "ScriptablePluginObject.h"
#include "../urlmon_compat.h"

//static
CWebBrowserPool* CPlugin::browserPool;

CPlugin::CPlugin(NPP pNPInstance) :
	m_pWebBrowser(NULL),
    m_pNPInstance(pNPInstance),
    m_bInitialized(FALSE),
    m_pScriptableObject(NULL),
	m_pWindowObject(NULL),
	m_pCallbackObject(NULL),
    m_hWnd(NULL),
	m_Progress(0),
	m_SecureLockIcon(0),
	lpOldProc(NULL) {

}

CPlugin::~CPlugin() {
	if(m_pWebBrowser)
		delete m_pWebBrowser;
    if (m_pScriptableObject)
        NPN_ReleaseObject(m_pScriptableObject);
}


NPBool CPlugin::Init(NPWindow* pNPWindow) {
    if(m_bInitialized) // FIXME: is this needed?
        return TRUE;

    if(pNPWindow == NULL)
        return FALSE;

    m_hWnd = (HWND)pNPWindow->window;
    if(m_hWnd == NULL)
        return FALSE;

	// TODO: check if we're under chrome:/// to ensure security
	GetPageURL();

    // subclass window so we can intercept window messages and
    // do our drawing to it
    lpOldProc = SubclassWindow(m_hWnd, (WNDPROC)PluginWinProc);

    // associate window with our CPlugin object so we can access
    // it in the window procedure
    SetWindowLongPtr(m_hWnd, GWL_USERDATA, (LONG_PTR)this);

	SetWindowLongPtr(m_hWnd, GWL_STYLE, GetWindowLongPtr(m_hWnd, GWL_STYLE)|WS_CLIPCHILDREN);
	SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, GetWindowLongPtr(m_hWnd, GWL_EXSTYLE)|WS_EX_CONTROLPARENT);

    m_Window = pNPWindow;
    m_bInitialized = TRUE;

	// get DOM window object
	// FIXME: we need a better and cleaner way to do this.
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, reinterpret_cast<void*>(&m_pWindowObject.p));


	// NOTE: we do not create the browser control here.
	//       we create it in Navigate() on demand, which in turns calls InitBrowser()

	// Tell the world that we're ready for use!
	if(m_pCallbackObject) {
		//FIXME: this won't be called :-(
		m_pCallbackObject.Invoke(m_pNPInstance, "ready", NULL, "v");
	}

	return TRUE;
}

void CPlugin::Destroy() {

	if(m_pWebBrowser) {
		// we have to Release() the COM pointer first
		// Otherwise, after DestroyWindow, it will become invalid.
		// m_pWebBrowser->EasyUnadvise(); // disconnect the sink
		static_cast<CComPtr<IWebBrowser2>*>(m_pWebBrowser)->Release();
		m_pWebBrowser->DestroyWindow();
		delete m_pWebBrowser;
		m_pWebBrowser = NULL;
	}

    // subclass it back
    SubclassWindow(m_hWnd, lpOldProc);
    m_hWnd = NULL;
    m_bInitialized = FALSE;
}

bool CPlugin::InitBrowser(const char* url) {

	CWebBrowser* existingBrowser = NULL;

	if(url && url[0] == '@') { // the URL is a browser_id
		// See if the browser id really points to a
		// valid pre-existing WebBrowser control.
		sscanf(url, "@%p", &existingBrowser);
		// check if the control is really in our pool to prevent random memory access
		POSITION pos = browserPool->Find(existingBrowser);
		if(pos != NULL) {
			browserPool->RemoveAt(pos); // remove it from the pool.
		}
		else
			existingBrowser = NULL;
	}

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	if(existingBrowser) { // if an existing brower control is assigned to our browser window
		// FIXME: check if its HWND is valid as well.
		m_pWebBrowser = existingBrowser;

		m_pWebBrowser->SetPlugin(this);
		m_pWebBrowser->SetParent(m_hWnd);
		m_pWebBrowser->SetWindowPos(HWND_BOTTOM, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
		m_pWebBrowser->ShowWindow(SW_SHOW);
		(*m_pWebBrowser)->put_Visible(VARIANT_TRUE);
	}
	else { // create a new browser control
		m_pWebBrowser = new CWebBrowser(this);
		if(m_pWebBrowser->Create(m_hWnd, rc)) {
			m_pWebBrowser->SetWindowPos(HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
			// the web browser control is successfully created, navigate to the URL
			Navigate(url);
		}
		else {
			delete m_pWebBrowser;
			m_pWebBrowser = NULL;
		}
	}
	return (m_pWebBrowser != NULL);
}


bool CPlugin::IsInitialized() {
    return m_bInitialized;
}

int16_t CPlugin::HandleEvent(void* event) {
    // this function is called in Mac only
    return 0;
}

NPObject *CPlugin::GetScriptableObject() {
    if (!m_pScriptableObject) {
		m_pScriptableObject = ScriptablePluginObject::New(m_pNPInstance);
    }
    if (m_pScriptableObject) {
        NPN_RetainObject(m_pScriptableObject);
    }
    return m_pScriptableObject;
}


/* static */ char* CPlugin::Bstr2Utf8(BSTR bstr) {
	// SysStringByteLen
	int bstrlen = SysStringLen(bstr) + 1;
	int buf_size = WideCharToMultiByte(CP_UTF8, 0, bstr, bstrlen, NULL, 0, NULL, FALSE);
	char* buf = reinterpret_cast<char*>(NPN_MemAlloc(buf_size));
	if(WideCharToMultiByte(CP_UTF8, 0, bstr, bstrlen, buf, buf_size, NULL, FALSE) == 0) {
		// failed
		buf[0] = '\0'; // FIXME: should we return NULL instead?
	}
	return buf;
}


// properties

void CPlugin::SetCallbacks(NPObject* callbacks) {
	m_pCallbackObject = callbacks;
}


char* CPlugin::GetUrl() {
	BSTR url;
	(*m_pWebBrowser)->get_LocationURL(&url);
	char* ret = Bstr2Utf8(url);
	SysFreeString(url);
	return ret;
}

char* CPlugin::GetTitle() {
	BSTR name;
	(*m_pWebBrowser)->get_LocationName(&name);
	char* ret = Bstr2Utf8(name);
	SysFreeString(name);
	return ret;
}

// utility methods

void CPlugin::UpdateLocation(BSTR url) {
	if(m_pCallbackObject) {
		// char* location = Bstr2Utf8(url);
		// FIXME: why url is "about:blank" even when there is a page loaded sometimes?
		char* location = GetUrl();
		m_pCallbackObject.Invoke(m_pNPInstance, "setLocation", NULL, "vs", location);
		NPN_MemFree(location);
	}
}

void CPlugin::UpdateTitle(BSTR text) {
	if(m_pCallbackObject) {
		static NPIdentifier setTitle_id;
		if(setTitle_id == 0)
			setTitle_id = NPN_GetStringIdentifier("setTitle");
		char* title = Bstr2Utf8(text);
		m_pCallbackObject.Invoke(m_pNPInstance, setTitle_id, NULL, "vs", title);
		NPN_MemFree(title);
	}
}

void CPlugin::UpdateProgress(long progress) {
	if(m_Progress != progress) {
		m_Progress = progress;

		if(m_pCallbackObject) {
			static NPIdentifier setProgress_id = 0;
			if(setProgress_id == 0)
				setProgress_id = NPN_GetStringIdentifier("setProgress");
			m_pCallbackObject.Invoke(m_pNPInstance, setProgress_id, NULL, "vi", (int)progress);
		}
	}
}

void CPlugin::UpdateStatusText(BSTR status) {
	// (*m_pWebBrowser)->get_StatusText();
	char* text = Bstr2Utf8(status);
	// NPN_Status(m_pNPInstance, text);
	if(m_pCallbackObject) {
		static NPIdentifier setStatusText_id = 0;
		if(setStatusText_id == 0)
			setStatusText_id = NPN_GetStringIdentifier("setStatusText");
		m_pCallbackObject.Invoke(m_pNPInstance, setStatusText_id, NULL, "vs", text);
	}
	NPN_MemFree(text);
}

void CPlugin::UpdateSecureLockIcon(long icon_id) {
	if(m_SecureLockIcon != icon_id) {
		// notify the javascript part
		m_SecureLockIcon = icon_id;
		if(m_pCallbackObject) {
			static NPIdentifier setSecurityIcon_id = 0;
			if(setSecurityIcon_id == 0)
				setSecurityIcon_id = NPN_GetStringIdentifier("setSecurityIcon");
			m_pCallbackObject.Invoke(m_pNPInstance, "setSecurityIcon", NULL, "vi", (int)icon_id);
		}
	}
}

void CPlugin::NewTab(const char* url) {
	if(m_pCallbackObject) {
		m_pCallbackObject.Invoke(m_pNPInstance, "newTab", NULL, "vs", url);
	}
}

void CPlugin::NewTab(CWebBrowser* newWebBrowser) {
	char url[sizeof(CWebBrowser*) * 2 + 8];
	sprintf(url, "@%p", newWebBrowser);
	NewTab(url);
}

void CPlugin::CloseTab() {
	if(m_pCallbackObject) {
		m_pCallbackObject.Invoke(m_pNPInstance, "closeTab", NULL, "v");
	}
}

// get the URL of the html page containing the plugin
CString CPlugin::GetPageURL(void)
{
	static NPIdentifier location_id = 0, href_id = 0;
	if(location_id == 0) {
		location_id = NPN_GetStringIdentifier("location");
		href_id = NPN_GetStringIdentifier("href");
	}

	NPVariant location_value;
	if(NPN_GetProperty(m_pNPInstance, m_pWindowObject.p, location_id, &location_value)) {
		NPObject *location = NPVARIANT_TO_OBJECT(location_value);
		NPVariant href_value;
		if(NPN_GetProperty( m_pNPInstance, location, href_id, &href_value)) {
			NPString href = NPVARIANT_TO_STRING(href_value);
			CString ret = href.UTF8Characters;
			NPN_ReleaseVariantValue(&href_value);
			return ret;
		}
		NPN_ReleaseVariantValue(&location_value);
	}
	return CString();
}


// static
void CPlugin::GlobalInit() {

	// CoInternetSetFeatureEnabled, enable some new features in newer IE
	HMODULE hUrlMonDll = LoadLibrary(_T("urlmon.dll"));
	if(hUrlMonDll) {
		typedef HRESULT (WINAPI *PCoInternetSetFeatureEnabled)(INTERNETFEATURELIST, DWORD, BOOL);
		PCoInternetSetFeatureEnabled pCoInternetSetFeatureEnabled = reinterpret_cast<PCoInternetSetFeatureEnabled>(GetProcAddress(hUrlMonDll, "CoInternetSetFeatureEnabled"));
		if(pCoInternetSetFeatureEnabled) {
			pCoInternetSetFeatureEnabled(FEATURE_WEBOC_POPUPMANAGEMENT, SET_FEATURE_ON_PROCESS, TRUE);
			pCoInternetSetFeatureEnabled(FEATURE_SECURITYBAND, SET_FEATURE_ON_PROCESS, TRUE);
			pCoInternetSetFeatureEnabled(FEATURE_LOCALMACHINE_LOCKDOWN, SET_FEATURE_ON_PROCESS, TRUE);
			pCoInternetSetFeatureEnabled(FEATURE_SAFE_BINDTOOBJECT, SET_FEATURE_ON_PROCESS, TRUE);
			// #define FEATURE_TABBED_BROWSING 19
			// pCoInternetSetFeatureEnabled((INTERNETFEATURELIST)FEATURE_TABBED_BROWSING, SET_FEATURE_ON_PROCESS, TRUE);	// IE7+
		}
		FreeLibrary(hUrlMonDll);
	}

	// create the web browser pool
	browserPool = new CWebBrowserPool();
	browserPool->Create(HWND_DESKTOP);
}

// static
void CPlugin::GlobalDestroy() {
	if(browserPool) {
		browserPool->DestroyWindow();
		delete browserPool; // FIXME: should we use delete this in FinalMessage?
		browserPool = NULL;
	}
}


// --------------------------------------------------------------------------------------------
// Following are Windows dirty jobs

/* static */ LRESULT CALLBACK CPlugin::PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_SIZE: {
        CPlugin * plugin = (CPlugin *)GetWindowLongPtr(hWnd, GWL_USERDATA);
        RECT rc;
        GetClientRect(hWnd, &rc);
		if(plugin->m_pWebBrowser)
			plugin->m_pWebBrowser->MoveWindow(0, 0, rc.right, rc.bottom);
		break;
	}
	case WM_ERASEBKGND: {
        CPlugin * plugin = (CPlugin *)GetWindowLongPtr(hWnd, GWL_USERDATA);
		if(plugin->m_pWebBrowser) // if web browser control is created
			return TRUE; // don't paint background
		break;
	}
	// case WM_PAINT:
	//	return 0;
	default:
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

CComModule comModule; // FIXME: how to replace this with new ATL module classes?
// CAtlWinModule *atlModule = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
        // DisableThreadLibraryCalls(hinstDLL);
        comModule.Init(NULL, hinstDLL, &LIBID_ATLLib);
        // Due to a bug of ATL, we should either pass our LIBID here, or
        // link to ATL dynamically. So we link atl dynamically.
        // http://support.microsoft.com/kb/832687/en-us
        // atlModule = new CAtlWinModule();
		// CAtlModule::m_libid = LIBID_ATLLib; // FIXME: is this workaround correct?
        break;
    case DLL_PROCESS_DETACH:
        comModule.Term();
		// delete atlModule;
        // atlModule = NULL;
        break;
    default:
        break;
    }
    return TRUE;
}
