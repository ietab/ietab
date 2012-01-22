//
// CustomClientSite.h: Custom OLE client site for the web browser control.
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

#include "customclientsite.h"

CCustomClientSite::CCustomClientSite(void) {
}

CCustomClientSite::~CCustomClientSite(void) {
	// ATLTRACE("destroy custom client site\n");
}


// IDocHostUIHandler
STDMETHODIMP CCustomClientSite::GetHostInfo(DOCHOSTUIINFO FAR* pInfo) {
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
STDMETHODIMP CCustomClientSite::QueryStatus( 
	/* [unique][in] */ const GUID *pguidCmdGroup,
	/* [in] */ ULONG cCmds,
	/* [out][in][size_is] */ OLECMD prgCmds[  ],
	/* [unique][out][in] */ OLECMDTEXT *pCmdText) {
	return pguidCmdGroup ? OLECMDERR_E_UNKNOWNGROUP : OLECMDERR_E_NOTSUPPORTED;
}

STDMETHODIMP CCustomClientSite::Exec( 
	/* [unique][in] */ const GUID *pguidCmdGroup,
	/* [in] */ DWORD nCmdID,
	/* [in] */ DWORD nCmdexecopt,
	/* [unique][in] */ VARIANT *pvaIn,
	/* [unique][out][in] */ VARIANT *pvaOut) {
	return OLECMDERR_E_NOTSUPPORTED;

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
