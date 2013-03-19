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

#pragma warning(disable:4996) // warning C4996: 'sprintf': This function or variable may be unsafe

#include <stdio.h>
#include "plugin.h"
#include <windows.h>
#include <windowsx.h>
#include "ScriptablePluginObject.h"
#include "../urlmon_compat.h"

#include "../chrom/chrom.h"

#define WM_IETAB_COMMAND	(WM_APP + 1)

enum IETabCommand {
	NEW_TAB,
	CLOSE_TAB,
	UPDATE_TITLE,
	UPDATE_STATUS_TEXT,
	UPDATE_LOCATION,
	UPDATE_PROGRESS,
	UPDATE_SECURE_LOCK_ICON,
	SWITCH_BACK_TO_FIREFOX,
	N_COMMANDS
};

// NPIdentifiers used for methods and properties
// properties
static NPIdentifier callbacks_id;
static NPIdentifier canBack_id;
static NPIdentifier canForward_id;
static NPIdentifier canRefresh_id;
static NPIdentifier canStop_id;
static NPIdentifier progress_id;
static NPIdentifier security_id;
static NPIdentifier url_id;
static NPIdentifier title_id;
static NPIdentifier statusText_id;
static NPIdentifier canCut_id;
static NPIdentifier canCopy_id;
static NPIdentifier canPaste_id;

// methods
static NPIdentifier goBack_id;
static NPIdentifier goForward_id;
static NPIdentifier navigate_id;
static NPIdentifier refresh_id;
static NPIdentifier stop_id;
static NPIdentifier saveAs_id;
static NPIdentifier print_id;
static NPIdentifier printPreview_id;
static NPIdentifier printSetup_id;
static NPIdentifier cut_id;
static NPIdentifier copy_id;
static NPIdentifier paste_id;
static NPIdentifier selectAll_id;
static NPIdentifier find_id;
static NPIdentifier viewSource_id;
static NPIdentifier focus_id;

static NPIdentifier sPluginType_id;

//static
CWebBrowserPool* CPlugin::browserPool;
CComModule comModule; // FIXME: how to replace this with new ATL module classes?
// CAtlWinModule *atlModule = NULL;

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
	m_lpOldProc(NULL) {

	m_ThreadId = GetCurrentThreadId();
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
    m_lpOldProc = SubclassWindow(m_hWnd, (WNDPROC)PluginWinProc);

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
	if(m_pCallbackObject && IsMainThread()) {
		//FIXME: this won't be called :-(
		m_pCallbackObject.Invoke(m_pNPInstance, "ready", NULL, "v");
	}

	/*
	DWORD current_thread = ::GetThreadId(GetCurrentThread());
	DWORD parent_thread = 0;
	HWND parent = m_hWnd;
	do {
		parent = ::GetParent(parent);
		parent_thread = ::GetWindowThreadProcessId(parent, NULL);
		if(parent_thread != current_thread) {
			AttachThreadInput(current_thread, parent_thread, TRUE);
			break;
		}
	}while(parent != NULL && parent != HWND_DESKTOP);
	*/

	return TRUE;
}

void CPlugin::Destroy() {

	if(m_pWebBrowser) {
		// m_pWebBrowser->EasyUnadvise(); // disconnect the sink
		m_pWebBrowser->SetPlugin(NULL);
		m_pWebBrowser->DestroyWindow();
		delete m_pWebBrowser;
		m_pWebBrowser = NULL;
	}

    // subclass it back
    SubclassWindow(m_hWnd, m_lpOldProc);
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
		m_pWebBrowser->GetIWebBrowser2()->put_Visible(VARIANT_TRUE);
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
	if(!m_pWebBrowser)
		return NULL;
	BSTR url;
	m_pWebBrowser->GetIWebBrowser2()->get_LocationURL(&url);
	char* ret = Bstr2Utf8(url);
	SysFreeString(url);
	return ret;
}

char* CPlugin::GetTitle() {
	if(!m_pWebBrowser)
		return NULL;
	BSTR name;
	m_pWebBrowser->GetIWebBrowser2()->get_LocationName(&name);
	char* ret = Bstr2Utf8(name);
	SysFreeString(name);
	return ret;
}

char* CPlugin::GetStatusText() {
	if(!m_pWebBrowser)
		return NULL;
/*
	BSTR status;
	(*m_pWebBrowser)->get_StatusText(&status);
	char* ret = Bstr2Utf8(status);
	SysFreeString(status);
*/
	// For security reason, newer versions of IE does not allow programs
	// to access status bar. So we used the stored one get from StatusChanged event
	// from the web browser control.
	char* ret = Bstr2Utf8(m_StatusText);
	return ret;
}


// utility methods

// For making an async call on main thread, ideally, NPN_PluginThreadAsyncCall() 
// should be used, but it's known to be problemaitc and does not work in some browsers.
// So we use Windows message queue here as suggested by the author of FireBreath framework.

void CPlugin::OnUpdateLocation() {
	if(m_pCallbackObject) {
		// char* location = Bstr2Utf8(url);
		// FIXME: why url is "about:blank" even when there is a page loaded sometimes?
		char* location = GetUrl();
		m_pCallbackObject.Invoke(m_pNPInstance, "setLocation", NULL, "vs", location);
		NPN_MemFree(location);
	}
}

void CPlugin::UpdateLocation(BSTR url) {
	PostMessage(m_hWnd, WM_IETAB_COMMAND, UPDATE_LOCATION, 0);
}

void CPlugin::OnUpdateTitle() {
	if(m_pCallbackObject) {
		static NPIdentifier setTitle_id;
		if(setTitle_id == 0)
			setTitle_id = NPN_GetStringIdentifier("setTitle");
		char* title = GetTitle();
		m_pCallbackObject.Invoke(m_pNPInstance, setTitle_id, NULL, "vs", title);
		NPN_MemFree(title);
	}
}

void CPlugin::UpdateTitle(BSTR text) {
	PostMessage(m_hWnd, WM_IETAB_COMMAND, UPDATE_TITLE, 0);
}

void CPlugin::OnUpdateProgress(long progress) {
	if(m_Progress != progress) {
		m_Progress = progress;

		if(m_pCallbackObject && IsMainThread()) {
			static NPIdentifier setProgress_id = 0;
			if(setProgress_id == 0)
				setProgress_id = NPN_GetStringIdentifier("setProgress");
			m_pCallbackObject.Invoke(m_pNPInstance, setProgress_id, NULL, "vi", (int)progress);
		}
	}
}

void CPlugin::UpdateProgress(long progress) {
	PostMessage(m_hWnd, WM_IETAB_COMMAND, UPDATE_PROGRESS, progress);
}

void CPlugin::OnUpdateStatusText() {
	if(m_pCallbackObject) {
		char* text = Bstr2Utf8(m_StatusText);
		// char* text = GetStatusText();
		static NPIdentifier setStatusText_id = 0;
		if(setStatusText_id == 0)
			setStatusText_id = NPN_GetStringIdentifier("setStatusText");
		m_pCallbackObject.Invoke(m_pNPInstance, setStatusText_id, NULL, "vs", text ? text : "");
		// NPN_Status(m_pNPInstance, text);
		NPN_MemFree(text);
	}
}

void CPlugin::UpdateStatusText(BSTR status) {
	// Calling back to Javascript directly here hangs Firefox.
	// We use PostMessage to schedule an async call
	m_StatusText = status;
	PostMessage(m_hWnd, WM_IETAB_COMMAND, UPDATE_STATUS_TEXT, 0);
}

void CPlugin::OnUpdateSecureLockIcon(long icon_id) {
	// notify the javascript part
	m_SecureLockIcon = icon_id;
	if(m_pCallbackObject) {
		static NPIdentifier setSecurityIcon_id = 0;
		if(setSecurityIcon_id == 0)
			setSecurityIcon_id = NPN_GetStringIdentifier("setSecurityIcon");
		m_pCallbackObject.Invoke(m_pNPInstance, "setSecurityIcon", NULL, "vi", (int)icon_id);
	}
}

void CPlugin::UpdateSecureLockIcon(long icon_id) {
	if(m_SecureLockIcon != icon_id)
		PostMessage(m_hWnd, WM_IETAB_COMMAND, UPDATE_SECURE_LOCK_ICON, icon_id);
}

void CPlugin::OnNewTab() {
	// TODO: I have no idea how to make this work properly.
}

void CPlugin::NewTab(const char* url) {
	if(m_pCallbackObject && IsMainThread()) {
		m_pCallbackObject.Invoke(m_pNPInstance, "newTab", NULL, "vs", url);
	}
}

void CPlugin::NewTab(CWebBrowser* newWebBrowser) {
	char url[sizeof(CWebBrowser*) * 2 + 8];
	sprintf(url, "@%p", newWebBrowser);
	NewTab(url);
}

void CPlugin::OnCloseTab() {
	if(m_pCallbackObject) {
		m_pCallbackObject.Invoke(m_pNPInstance, "closeTab", NULL, "v");
	}
}

void CPlugin::CloseTab() {
	PostMessage(m_hWnd, WM_IETAB_COMMAND, CLOSE_TAB, 0);
}

void CPlugin::OnSwitchBackToFirefox() {
	if(m_pCallbackObject) {
		m_pCallbackObject.Invoke(m_pNPInstance, "switchBackToFirefox", NULL, "v");
	}
}

void CPlugin::SwitchBackToFirefox() {
	PostMessage(m_hWnd, WM_IETAB_COMMAND, SWITCH_BACK_TO_FIREFOX, 0);
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

	RegisterIdentifiers();

	// create the web browser pool
	browserPool = new CWebBrowserPool();
	browserPool->Create(HWND_DESKTOP);

	// create our item in IE popup menu via registry
	CString modulePath;
	::GetModuleFileName((HMODULE)comModule.GetResourceInstance(), modulePath.GetBuffer(_MAX_PATH), _MAX_PATH);
	modulePath.ReleaseBuffer();

	CRegKey reg;
	// FIXME: We should create a localized key name here so the user can get a translated string.
	if(reg.Create(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\MenuExt\\Switch To Firefox"), 
		REG_NONE, REG_OPTION_VOLATILE) ==  ERROR_SUCCESS) {
		// reg.SetStringValue(NULL, "res://" + modulePath + _T("/switchback.html"));
		modulePath = modulePath.Mid(0, modulePath.ReverseFind('\\'));
		reg.SetStringValue(NULL, modulePath + _T("\\switchback.html"));
		reg.SetDWORDValue(_T("Contexts"), 0x1);	// default
		reg.Close();
	}
}

// static
void CPlugin::GlobalDestroy() {
	if(browserPool) {
		browserPool->DestroyWindow();
		delete browserPool; // FIXME: should we use delete this in FinalMessage?
		browserPool = NULL;
	}

	// remove our item from IE popup menu
	CRegKey reg;
	if(reg.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\MenuExt")) ==  ERROR_SUCCESS) {
		reg.DeleteSubKey(_T("Switch To Firefox"));
		reg.Close();
	}
}

//------------------------------------------------------------------------------------------
// NPObject functions redirected from ScriptablePluginObject

bool CPlugin::HasMethod(NPIdentifier name) {
    return (name == goBack_id ||
            name == goForward_id ||
            name == navigate_id ||
            name == refresh_id ||
            name == stop_id ||
            name == saveAs_id ||
            name == print_id ||
            name == printPreview_id ||
            name == printSetup_id ||
            name == cut_id ||
            name == copy_id ||
            name == paste_id ||
            name == selectAll_id ||
            name == find_id ||
            name == viewSource_id ||
            name == focus_id);
}

bool CPlugin::HasProperty(NPIdentifier name) {
    return (name == callbacks_id ||
            name == canBack_id ||
            name == canForward_id ||
            name == canRefresh_id ||
            name == canStop_id ||
            name == canCut_id ||
            name == canCopy_id ||
            name == canPaste_id ||
            name == progress_id ||
            name == security_id ||
            name == url_id ||
            name == title_id||
			name == statusText_id ||
            name == sPluginType_id);

}

bool CPlugin::GetProperty(NPIdentifier name, NPVariant *result) {
    VOID_TO_NPVARIANT(*result);

    if(name == callbacks_id) {
		NPObject* target = GetCallbacks();
		if(target)
			NPN_RetainObject(target);
		OBJECT_TO_NPVARIANT(target, *result);
    }
    else if(name == canBack_id) {
		BOOLEAN_TO_NPVARIANT(GetCanBack(), *result);
    }
    else if(name == canForward_id) {
		BOOLEAN_TO_NPVARIANT(GetCanForward(), *result);
    }
    else if(name == canRefresh_id) {
		BOOLEAN_TO_NPVARIANT(GetCanRefresh(), *result);
    }
    else if(name == canStop_id) {
		BOOLEAN_TO_NPVARIANT(GetCanStop(), *result);
    }
    else if(name == canCut_id) {
        BOOLEAN_TO_NPVARIANT(GetCanCut(), *result);
    }
    else if(name == canCopy_id) {
		BOOLEAN_TO_NPVARIANT(GetCanCopy(), *result);
    }
    else if(name == canPaste_id) {
		BOOLEAN_TO_NPVARIANT(GetCanPaste(), *result);
    }
    else if(name == progress_id) {
		INT32_TO_NPVARIANT(GetProgress(), *result);
    }
    else if(name == security_id) {
		INT32_TO_NPVARIANT(GetSecurity(), *result);
    }
    else if(name == url_id) {
		char* url = GetUrl();
		if(url)
	        STRINGZ_TO_NPVARIANT(url, *result);
    }
    else if(name == title_id) {
		char* title = GetTitle();
		if(title)
	        STRINGZ_TO_NPVARIANT(title, *result);
    }
    else if(name == statusText_id) {
		char* status = GetStatusText();
		if(status)
	        STRINGZ_TO_NPVARIANT(status, *result);
    }
    else if (name == sPluginType_id) {
#if 0
        // FIXME: seriously, what on earth is this for?
        NPObject *myobj =
            NPN_CreateObject(m_pNPInstance, GET_NPOBJECT_CLASS(ConstructablePluginObject));
        if (!myobj) {
            return false;
        }
        OBJECT_TO_NPVARIANT(myobj, *result);
#endif
		return false;
    }
    else {
        return false;
    }
    return true;
}

bool CPlugin::SetProperty(NPIdentifier name, const NPVariant *value) {
    if(name == callbacks_id) {
		NPObject* target = NPVARIANT_TO_OBJECT(*value);
		SetCallbacks(target);
    }
    else if(name == url_id) {
		const NPString url = NPVARIANT_TO_STRING(*value);
		Navigate((const char*)url.UTF8Characters);
    }
	else {
	    return false;
	}
	return true;
}

bool CPlugin::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
    if(name == goBack_id) {
		GoBack();
    }
    else if(name == goForward_id) {
		GoForward();
    }
    else if(name == navigate_id) {
		if(argCount >= 1) {
			if(NPVARIANT_IS_STRING(args[0])) {
				Navigate(NPVARIANT_TO_STRING(args[0]).UTF8Characters);
				BOOLEAN_TO_NPVARIANT(true, *result);
			}
		}
		return true;
    }
    else if(name == refresh_id) {
		Refresh();
    }
    else if(name == stop_id) {
		Stop();
    }
    else if(name == saveAs_id) {
		SaveAs();
    }
    else if(name == print_id) {
		Print();
    }
    else if(name == printPreview_id) {
		PrintPreview();
	}
    else if(name == printSetup_id) {
		PrintSetup();
    }
    else if(name == cut_id) {
		Cut();
    }
    else if(name == copy_id) {
		Copy();
    }
    else if(name == paste_id) {
		Paste();
    }
    else if(name == selectAll_id) {
		SelectAll();
    }
    else if(name == find_id) {
		Find();
    }
    else if(name == viewSource_id) {
		ViewSource();
    }
    else if(name == focus_id) {
		Focus();
    }
    else {
        return false;
    }
    return true;
}

bool CPlugin::InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result) {
    VOID_TO_NPVARIANT(*result);
    return true;
}

void CPlugin::RegisterIdentifiers() {
    // properties
    callbacks_id = NPN_GetStringIdentifier("callbacks");
    canBack_id = NPN_GetStringIdentifier("canBack");
    canForward_id = NPN_GetStringIdentifier("canForward");
    canRefresh_id = NPN_GetStringIdentifier("canRefresh");
    canStop_id = NPN_GetStringIdentifier("canStop");
    canCut_id = NPN_GetStringIdentifier("canCut");
    canCopy_id = NPN_GetStringIdentifier("canCopy");
    canPaste_id = NPN_GetStringIdentifier("canPaste");
    progress_id = NPN_GetStringIdentifier("progress");
    security_id = NPN_GetStringIdentifier("security");
    url_id = NPN_GetStringIdentifier("url");
    title_id = NPN_GetStringIdentifier("title");
	statusText_id = NPN_GetStringIdentifier("statusText");

    // methods
    goBack_id = NPN_GetStringIdentifier("goBack");
    goForward_id = NPN_GetStringIdentifier("goForward");
    navigate_id = NPN_GetStringIdentifier("navigate");
    refresh_id = NPN_GetStringIdentifier("refresh");
    stop_id = NPN_GetStringIdentifier("stop");
    saveAs_id = NPN_GetStringIdentifier("saveAs");
    print_id = NPN_GetStringIdentifier("print");
    printPreview_id = NPN_GetStringIdentifier("printPreview");
    printSetup_id = NPN_GetStringIdentifier("printSetup");
    cut_id = NPN_GetStringIdentifier("cut");
    copy_id = NPN_GetStringIdentifier("copy");
    paste_id = NPN_GetStringIdentifier("paste");
    selectAll_id = NPN_GetStringIdentifier("selectAll");
    find_id = NPN_GetStringIdentifier("find");
    viewSource_id = NPN_GetStringIdentifier("viewSource");
    focus_id = NPN_GetStringIdentifier("focus");

    // sPluginType_id = NPN_GetStringIdentifier("PluginType");
}


// --------------------------------------------------------------------------------------------
// Following are Windows dirty jobs

/* static */ LRESULT CALLBACK CPlugin::PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    CPlugin * plugin = (CPlugin *)GetWindowLongPtr(hWnd, GWL_USERDATA);
    switch (msg) {
	case WM_SIZE: {
        RECT rc;
        GetClientRect(hWnd, &rc);
		if(plugin->m_pWebBrowser)
			plugin->m_pWebBrowser->MoveWindow(0, 0, rc.right, rc.bottom);
		break;
	}
	case WM_ERASEBKGND: {
		if(plugin->m_pWebBrowser) // if web browser control is created
			return TRUE; // don't paint background
		break;
	}
	case WM_IETAB_COMMAND: {
		switch(wParam) {
		case NEW_TAB:
			plugin->OnNewTab();
			break;
		case CLOSE_TAB:
			plugin->OnCloseTab();
			break;
		case UPDATE_TITLE:
			plugin->OnUpdateTitle();
			break;
		case UPDATE_STATUS_TEXT:
			plugin->OnUpdateStatusText();
			break;
		case UPDATE_LOCATION:
			plugin->OnUpdateLocation();
			break;
		case UPDATE_PROGRESS:
			plugin->OnUpdateProgress(lParam);
			break;
		case UPDATE_SECURE_LOCK_ICON:
			plugin->OnUpdateSecureLockIcon(lParam);
			break;
		case SWITCH_BACK_TO_FIREFOX:
			plugin->OnSwitchBackToFirefox();
			break;
		}
		break;
	}
	case WM_SETFOCUS:
		// Focus the browser control if needed
		plugin->Focus();
		break;
	default:
        break;
    }
    // return DefWindowProc(hWnd, msg, wParam, lParam);
	return CallWindowProc(plugin->m_lpOldProc, hWnd, msg, wParam, lParam);
}

Hook gTranslateMessageHook;
static bool apiHooked = false;

typedef BOOL (WINAPI *TranslateMessageProc)(const MSG *lpMsg);

// Firefox does not handle TranselateAccelerators so some keys do not
// work in IE Tab WebBrowser control. Normally, TranslateAccelerator() should
// be called just prior to TranslateMessage(). So we use API hook technique
// here and hijack TranslateMessage calls.
// Special thanks to Chrom library created by Raja Jamwal 2011, <www.experiblog.co.cc> <linux1@zoho.com>.
BOOL WINAPI TranslateMessageHook(const MSG *lpMsg) {
	// reset hooks, this will replace the jump instruction to original data

	// let our browser controls filter the message first.
	BOOL ret = CWebBrowser::PreTranslateMessage((MSG*)lpMsg);
	if(!ret) {
		// if we don't translate any accelerators, call the original TranslateMessage().
	    gTranslateMessageHook.Reset();
		TranslateMessageProc oldTranslateMessage;
		oldTranslateMessage = (TranslateMessageProc)gTranslateMessageHook.original_function;
		ret = oldTranslateMessage(lpMsg);
		// again place the jump instruction on the original function
	    gTranslateMessageHook.Place_Hook();
	}
	return ret;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
        // DisableThreadLibraryCalls(hinstDLL);
		if(!apiHooked) {
			gTranslateMessageHook.Initialize("TranslateMessage", _T("user32.dll"), TranslateMessageHook);
			// Write jump instruction on original function address
			gTranslateMessageHook.Start();
			apiHooked = true;
		}

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
