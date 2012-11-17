//
// WebBrowserHost.cpp: Subclass CAxHostWindow to create a specialized
// control host to provide IOleCommandTarget and IDocHostUIHandler.
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

#include "WebBrowserHost.h"
#include <atlstr.h>
#include <MLang.h>
#include <Shlguid.h>
#include <Mshtmcid.h>

// menu id for our own menu item
#define ID_SWITCHBACK		45000 // there is no document about what IDs are used by IE, so we pick a relatively bigger one.


CWebBrowserHost::CWebBrowserHost(void) {
}

CWebBrowserHost::~CWebBrowserHost(void) {
}

// IDocHostUIHandler
STDMETHODIMP CWebBrowserHost::GetHostInfo(DOCHOSTUIINFO FAR* pInfo) {
	if(pInfo != NULL) {
		pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER
			|DOCHOSTUIFLAG_THEME
			|DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE
			|DOCHOSTUIFLAG_LOCAL_MACHINE_ACCESS_CHECK
			|DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION;
		pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
	}
	return S_OK;
}


// IOleCommandTarget
STDMETHODIMP CWebBrowserHost::QueryStatus( 
	/* [unique][in] */ const GUID *pguidCmdGroup,
	/* [in] */ ULONG cCmds,
	/* [out][in][size_is] */ OLECMD prgCmds[],
	/* [unique][out][in] */ OLECMDTEXT *pCmdText) {
	return pguidCmdGroup ? OLECMDERR_E_UNKNOWNGROUP : OLECMDERR_E_NOTSUPPORTED;
}

STDMETHODIMP CWebBrowserHost::Exec( 
	/* [unique][in] */ const GUID *pguidCmdGroup,
	/* [in] */ DWORD nCmdID,
	/* [in] */ DWORD nCmdexecopt,
	/* [unique][in] */ VARIANT *pvaIn,
	/* [unique][out][in] */ VARIANT *pvaOut) {

	HRESULT hr = pguidCmdGroup ? OLECMDERR_E_UNKNOWNGROUP : OLECMDERR_E_NOTSUPPORTED;
	if(pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler)) {
		if(nCmdID == OLECMDID_SHOWSCRIPTERROR) {
			hr = S_OK;
			// http://support.microsoft.com/default.aspx?scid=kb;en-us;261003
			(*pvaOut).vt = VT_BOOL;
			// Continue running scripts on the page.
			(*pvaOut).boolVal = VARIANT_TRUE;
		}
	}
	return hr;
}

HRESULT CWebBrowserHost::AxCreateControlLicEx(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, 
	IUnknown** ppUnkContainer, IUnknown** ppUnkControl, REFIID iidSink, IUnknown* punkSink, BSTR bstrLic)
{
	AtlAxWinInit();
	HRESULT hr;
	CComPtr<IUnknown> spUnkContainer;
	CComPtr<IUnknown> spUnkControl;

	hr = CWebBrowserHost::_CreatorClass::CreateInstance(NULL, __uuidof(IUnknown), (void**)&spUnkContainer);
	if (SUCCEEDED(hr)) {
		CComPtr<IAxWinHostWindowLic> pAxWindow;
		spUnkContainer->QueryInterface(__uuidof(IAxWinHostWindow), (void**)&pAxWindow);
		CComBSTR bstrName(lpszName);
		hr = pAxWindow->CreateControlLicEx(bstrName, hWnd, pStream, &spUnkControl, iidSink, punkSink, bstrLic);
	}
	if (ppUnkContainer != NULL)	{
		if (SUCCEEDED(hr)) {
			*ppUnkContainer = spUnkContainer.p;
			spUnkContainer.p = NULL;
		}
		else
			*ppUnkContainer = NULL;
	}
	if (ppUnkControl != NULL) {
		if (SUCCEEDED(hr)) {
			*ppUnkControl = SUCCEEDED(hr) ? spUnkControl.p : NULL;
			spUnkControl.p = NULL;
		}
		else
			*ppUnkControl = NULL;
	}
	return hr;
}

STDMETHODIMP CWebBrowserHost::ShowContextMenu(DWORD dwID, POINT FAR* ppt, IUnknown* pcmdTarget, IDispatch* pdispObject) {

#if 0
	// we tried to implement a custom context menu, but failed. :-(
	// documentation on newer versions of IE is lacking...
	// So, let's do it with the registry dirty trick.

	#define IDR_BROWSE_CONTEXT_MENU  24641
	#define SHDVID_GETMIMECSETMENU   27
	#define SHDVID_ADDMENUEXTENSIONS 53
	#define IDM_EXTRA_ITEM			 6047

	if(dwID != CONTEXT_MENU_DEFAULT) // we only touch the default context menu
		return E_NOTIMPL;

	HMENU topMenu = NULL;

	// Try to load the menu from ieframe.dll.ui first (IE 7/8/9)

	// Get locale name:
	// http://qualapps.blogspot.com/2011/10/convert-locale-name-to-lcid-in-c.html
	// http://stackoverflow.com/questions/7749999/converting-lcid-to-language-string
	// Location of ieframe.dll.ui
	// http://www.cfanclub.net/article.php?itemid-41488-type-news.html

	CComPtr<IMultiLanguage> iLang;
	if(SUCCEEDED(iLang.CoCreateInstance(CLSID_CMultiLanguage))) {
		LCID lcid = GetUserDefaultLCID();
		BSTR locale = NULL;
		if(SUCCEEDED(iLang->GetRfc1766FromLcid(lcid, &locale))) {
			CString path;
			path.Format(_T("%s\\ieframe.dll.mui"), locale);
			HMODULE ieframe = LoadLibrary(path);
			if(ieframe != NULL) {
				topMenu = LoadMenu((HINSTANCE)ieframe, MAKEINTRESOURCE(IDR_BROWSE_CONTEXT_MENU));
				FreeLibrary(ieframe);
			}
			::SysFreeString(locale);
		}
	}

	if(topMenu == NULL) { // load menu failed, try shdoclc.dll instead (IE 6)
		HMODULE shdoclc = LoadLibrary(_T("shdoclc.dll"));
		if(shdoclc != NULL) {
			topMenu = LoadMenu((HINSTANCE)shdoclc, MAKEINTRESOURCE(IDR_BROWSE_CONTEXT_MENU));
			FreeLibrary(shdoclc);
		}
	}

	if(topMenu == NULL)
		return E_NOTIMPL;

	HWND hwnd;
	CComPtr<IOleCommandTarget> spCT;
	CComPtr<IOleWindow> spWnd;
	if( SUCCEEDED(pcmdTarget->QueryInterface(IID_IOleCommandTarget, (void**)&spCT)) && 
		SUCCEEDED(pcmdTarget->QueryInterface(IID_IOleWindow, (void**)&spWnd)) &&
		SUCCEEDED(spWnd->GetWindow(&hwnd))) {

		HMENU menu = GetSubMenu(topMenu, dwID);

		CComVariant var;
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof(mii);
		mii.fMask  = MIIM_SUBMENU;
		// Get the language submenu.
		if(SUCCEEDED(spCT->Exec(&CGID_ShellDocView, SHDVID_GETMIMECSETMENU, 0, NULL, &var))) {
			mii.hSubMenu = (HMENU)var.byref;
			// Add language submenu to Encoding context item.
			SetMenuItemInfo(menu, IDM_LANGUAGE, FALSE, &mii);
		}

		// Insert Shortcut Menu Extensions from registry.
		CComVariant var1, var2(dwID);
		V_VT(&var1) = VT_INT_PTR;
		V_BYREF(&var1) = menu;
		spCT->Exec(&CGID_ShellDocView, SHDVID_ADDMENUEXTENSIONS, 0, &var1, &var2);

		// Add our own menu item
		::InsertMenu(menu, 0, MF_ENABLED|MF_BYPOSITION, ID_SWITCHBACK, _T("Switch To Firefox"));
		::EnableMenuItem(menu, ID_SWITCHBACK, MF_BYCOMMAND|MF_ENABLED);

		// Remove the item that produces the exta separator
		::DeleteMenu(menu, IDM_EXTRA_ITEM, MF_BYCOMMAND);

		// Show shortcut menu.
		int id = ::TrackPopupMenu(menu,
							TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD,
							ppt->x,	ppt->y,	0, hwnd, (RECT*)NULL);
		if(id == ID_SWITCHBACK) {
			ATLTRACE("switch to firefox\n");
		}
		else {
			// Send selected shortcut menu item command to shell.
			LRESULT lr = ::SendMessage(hwnd, WM_COMMAND, id, NULL);
		}
		DestroyMenu(topMenu);
		return S_OK;
	} 
	DestroyMenu(topMenu);
#endif
	return E_NOTIMPL;
}
