//
// WebBrowserPool.h: A pool of newly created CWebBrowser controls
//                   This is mainly used by CWebBrowser::OnNewWindow().
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

#include <atlcoll.h> // for CAtlList
#include <atlstr.h>
#pragma once
#include <atlwin.h>
#include <cstdio>

class CWebBrowser;

typedef CWinTraits<WS_OVERLAPPEDWINDOW&~WS_VISIBLE> CWebBrowserPoolTraits;

struct CWebBrowserPoolItem {

	CWebBrowserPoolItem(CWebBrowser* webBrowser = NULL, CString url = CString()):
		m_URL(url),
		m_WebBrowser(webBrowser),
		m_WaitTime(0) {
	}

	CString m_URL;
	CWebBrowser* m_WebBrowser;
	int m_WaitTime;
};

class CWebBrowserPool: public CWindowImpl<CWebBrowserPool, CWindow, CWebBrowserPoolTraits> {

public:
	CWebBrowserPool(void);
	~CWebBrowserPool(void);

	POSITION Find(CString url);
	POSITION Find(CWebBrowser* webBrowser);

	CWebBrowser* AddNew(CString url);

	CWebBrowserPoolItem* GetAt(POSITION pos) {
		return m_Pool.GetAt(pos);
	}

	void RemoveAt(POSITION pos);

private:

	enum {
		TIMER_ID = 100,
		TIMEOUT = 10
	};

	BEGIN_MSG_MAP(CWebBrowser)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	CAtlList<CWebBrowserPoolItem*> m_Pool;
	bool m_HasTimer;
};

