//
// WebBrowserPool.cpp: A pool of newly created CWebBrowser controls
//                     This is mainly used by CWebBrowser::OnNewWindow().
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

#include "WebBrowserPool.h"
#include "WebBrowser.h"

CWebBrowserPool::CWebBrowserPool(void) {
}

CWebBrowserPool::~CWebBrowserPool(void) {
}

CWebBrowser* CWebBrowserPool::AddNew() {
	// create a new web browser control
	CWebBrowser* newWebBrowser = new CWebBrowser(NULL);
	RECT rc = {0};
	HWND hwnd = newWebBrowser->Create(m_hWnd, rc);
	if(hwnd) {
		m_Pool.AddTail(newWebBrowser);
	}
	else {
		delete newWebBrowser;
		newWebBrowser = NULL;
	}
	return newWebBrowser;
}

LRESULT CWebBrowserPool::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// FIXME: destroy all remaining orphaned web browser controls here.
	return DefWindowProc(uMsg, wParam, lParam);
}
