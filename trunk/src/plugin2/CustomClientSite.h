#pragma once
#include <atlbase.h>
#include <atlcom.h>
#include "oleidl.h"
#include <mshtmhst.h>

class CCustomClientSite :
	public CComObjectRoot,
	public IOleClientSite,
	public IDocHostUIHandler,
	public IOleCommandTarget,
	public IServiceProviderImpl<CCustomClientSite> {
public:
	CCustomClientSite(void);
	virtual ~CCustomClientSite(void);

	BEGIN_COM_MAP(CCustomClientSite)
		COM_INTERFACE_ENTRY(IOleClientSite)
		COM_INTERFACE_ENTRY(IDocHostUIHandler)
		COM_INTERFACE_ENTRY(IOleCommandTarget)
		COM_INTERFACE_ENTRY(IServiceProvider)
	END_COM_MAP()

	BEGIN_SERVICE_MAP(CCustomClientSite)
	END_SERVICE_MAP()

	// IOleClientSite
	STDMETHOD(SaveObject)(void) {
		return E_NOTIMPL;
	}
	STDMETHOD(GetMoniker)(DWORD nAssign, DWORD nWhichMoniker, IMoniker **ppMoniker) {
		return E_NOTIMPL;
	}
	STDMETHOD(GetContainer)(IOleContainer **ppContainer) {
		*ppContainer = NULL;
		return E_NOINTERFACE;
	}
	STDMETHOD(ShowObject)(void) {
		return S_OK;
	}
	STDMETHOD(OnShowWindow)(BOOL fShow) {
		return S_OK;
	}
	STDMETHOD(RequestNewObjectLayout)(void) {
		return E_NOTIMPL;
	}

	// IDocHostUIHandler
	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO FAR* pInfo);
	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID) {
		return S_FALSE;
	}
	STDMETHOD(GetExternal)(IDispatch** ppDispatch) {
		return E_NOTIMPL;
	}
	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT FAR* ppt, IUnknown FAR* pcmdtReserved, IDispatch FAR* pdispReserved) { 
		return S_FALSE;
	}
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

};
