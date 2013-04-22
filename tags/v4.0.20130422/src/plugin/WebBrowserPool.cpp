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

CWebBrowserPool::CWebBrowserPool(void):
	m_HasTimer(false) {
}

CWebBrowserPool::~CWebBrowserPool(void) {
}

CWebBrowser* CWebBrowserPool::AddNew(CString url) {
	CWebBrowser* newWebBrowser = new CWebBrowser(NULL);
	// create a new web browser control
	RECT rc = {0};
	HWND hwnd = newWebBrowser->Create(m_hWnd, rc);
	if(hwnd) {
		CWebBrowserPoolItem* item = new CWebBrowserPoolItem(newWebBrowser, url);
		m_Pool.AddTail(item);

		// Set a timeout to remove unused browser controls
		if(!m_HasTimer) {
			SetTimer(TIMER_ID, 1000, NULL);
			m_HasTimer = true;
		}
	}
	else {
		delete newWebBrowser;
		newWebBrowser = NULL;
	}
	return newWebBrowser;
}

POSITION CWebBrowserPool::Find(CString url) {
	for(POSITION pos = m_Pool.GetHeadPosition(); pos; m_Pool.GetNext(pos)) {
		CWebBrowserPoolItem* item = m_Pool.GetAt(pos);
		if(item->m_URL == url)
			return pos;
	}
	return NULL;
}

POSITION CWebBrowserPool::Find(CWebBrowser* webBrowser) {
	for(POSITION pos = m_Pool.GetHeadPosition(); pos; m_Pool.GetNext(pos)) {
		CWebBrowserPoolItem* item = m_Pool.GetAt(pos);
		if(item->m_WebBrowser == webBrowser)
			return pos;
	}
	return NULL;
}

void CWebBrowserPool::RemoveAt(POSITION pos) {
	m_Pool.RemoveAt(pos);
	if(m_Pool.IsEmpty() && m_HasTimer) {
		KillTimer(TIMER_ID);
		m_HasTimer = false;
	}
}


LRESULT CWebBrowserPool::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(wParam == TIMER_ID) {
		POSITION next = NULL;
		for(POSITION pos = m_Pool.GetHeadPosition(); pos; pos = next) {
			CWebBrowserPoolItem* item = m_Pool.GetAt(pos);
			next = pos;
			m_Pool.GetNext(next);
			++item->m_WaitTime;
			if(item->m_WaitTime > TIMEOUT) {
				//the item does not seem to be picked up by anyone.
				// Let's destroy it!
				// item->m_WebBrowser->DestroyWindow();
				delete item->m_WebBrowser;
				m_Pool.RemoveAt(pos);
			}
		}
	}
	return 0;
}

LRESULT CWebBrowserPool::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// FIXME: destroy all remaining orphaned web browser controls here.
	if(m_HasTimer) {
		KillTimer(TIMER_ID);
		m_HasTimer = false;
	}
	return DefWindowProc(uMsg, wParam, lParam);
}
