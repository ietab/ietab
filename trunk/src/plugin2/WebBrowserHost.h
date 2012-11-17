//
// WebBrowserHost.h: Subclass CAxHostWindow to create a specialized
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

#pragma once

#include <atlbase.h>
#include <atlhost.h>


class ATL_NO_VTABLE CWebBrowserHost :
	//public CComCoClass<CWebBrowserHost , &CLSID_NULL>,
	public CAxHostWindow,
	public IOleCommandTarget
{
public:
	CWebBrowserHost(void);
	~CWebBrowserHost(void);

	// DECLARE_WND_SUPERCLASS(_T("IETabHost"), CAxWindow::GetWndClassName())
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	DECLARE_NO_REGISTRY()
	DECLARE_POLY_AGGREGATABLE(CWebBrowserHost)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CWebBrowserHost)
		COM_INTERFACE_ENTRY(IDocHostUIHandler)
		// COM_INTERFACE_ENTRY(IOleCommandTarget)
		COM_INTERFACE_ENTRY_CHAIN(CAxHostWindow)
	END_COM_MAP()

	// IDocHostUIHandler
	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO FAR* pInfo);
	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID) {
		return E_NOTIMPL;
	}
	STDMETHOD(GetExternal)(IDispatch** ppDispatch) {
		return E_NOTIMPL;
	}

	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT FAR* ppt, IUnknown* pcmdTarget, IDispatch* pdispObject);

	STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject FAR* pActiveObject,
		IOleCommandTarget FAR* pCommandTarget,
		IOleInPlaceFrame  FAR* pFrame,
		IOleInPlaceUIWindow FAR* pDoc) {
			return S_FALSE;
	}
	STDMETHOD(HideUI)(void) {
		return S_OK;
	}
	STDMETHOD(UpdateUI)(void) {
		return S_OK;
	}
	STDMETHOD(EnableModeless)(BOOL fEnable) {
		return E_NOTIMPL;
	}
	STDMETHOD(OnDocWindowActivate)(BOOL fActivate) {
		return E_NOTIMPL;
	}
	STDMETHOD(OnFrameWindowActivate)(BOOL fActivate) {
		return E_NOTIMPL;
	}
	STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow FAR* pUIWindow, BOOL fRameWindow) {
		return E_NOTIMPL;
	}
	STDMETHOD(GetOptionKeyPath)(LPOLESTR FAR* pchKey, DWORD dw) {
		return S_FALSE;
	}
	STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget) {
		return E_NOTIMPL;
	}
	STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut) {
		* ppchURLOut = NULL;
		return S_FALSE;
	}
	STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet) {
		*ppDORet = NULL;
		return S_FALSE;
	}

	// IOleCommandTarget
	STDMETHOD(QueryStatus)(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
	STDMETHOD(Exec)(
		/* [unique][in] */ const GUID *pguidCmdGroup,
		/* [in] */ DWORD nCmdID,
		/* [in] */ DWORD nCmdexecopt,
		/* [unique][in] */ VARIANT *pvaIn,
		/* [unique][out][in] */ VARIANT *pvaOut);

	static HRESULT CWebBrowserHost::AxCreateControlLicEx(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, 
		IUnknown** ppUnkContainer, IUnknown** ppUnkControl, REFIID iidSink, IUnknown* punkSink, BSTR bstrLic);

};

