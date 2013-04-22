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
	return E_NOTIMPL;
}
