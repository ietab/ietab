const Cc = Components.classes;
const Ci = Components.interfaces;
Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

var IeTabFilter = {
	IETAB_CHROME_URL: "chrome://ietab/content/ietab.xul#",
	isIeTabURL: function(url) {
		if (!url) return false;
		return (url.indexOf(this.IETAB_CHROME_URL) == 0);
	},
	getIeTabURL: function(url) {
		if (this.isIeTabURL(url)) return url;
		if (/^file:\/\/.*/.test(url)) try { url = decodeURI(url).substring(8).replace(/\//g, "\\"); }catch(e){}
		return this.IETAB_CHROME_URL + encodeURI(url);
	},
	getBoolPref: function(prefName, defval) {
		var result = defval;
		var prefservice = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefService);
		var prefs = prefservice.getBranch("extensions.");
		if (prefs.getPrefType(prefName) == prefs.PREF_BOOL) {
			 try { result = prefs.getBoolPref(prefName); }catch(e){}
		}
		return(result);
	},
	getStrPref: function(prefName, defval) {
		var result = defval;
		var prefservice = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefService);
		var prefs = prefservice.getBranch("extensions.");
		if (prefs.getPrefType(prefName) == prefs.PREF_STRING) {
			 try { result = prefs.getComplexValue(prefName, Ci.nsISupportsString).data; }catch(e){}
		}
		return(result);
	},
	setStrPref: function(prefName, value) {
		var prefservice = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefService);
		var prefs = prefservice.getBranch("extensions.");
		var sString = Cc["@mozilla.org/supports-string;1"].createInstance(Ci.nsISupportsString);
		sString.data = value;
		try { prefs.setComplexValue(prefName, Ci.nsISupportsString, sString); } catch(e){}
	},
	isFilterEnabled: function() {
		return (this.getBoolPref("ietab.filter", true));
	},
	getPrefFilterList: function() {
		var s = this.getStrPref("ietab.filterlist", null);
		return (s ? s.split(" ") : "");
	},
	setPrefFilterList: function(list) {
		this.setStrPref("ietab.filterlist", list.join(" "));
	},
	isMatchURL: function(url, pattern) {
		if ((!pattern) || (pattern.length==0)) return false;
		var retest = /^\/(.*)\/$/.exec(pattern);
		if (retest) {
			pattern = retest[1];
		} else {
			pattern = pattern.replace(/\\/g, "/");
			var m = pattern.match(/^(.+:\/\/+[^\/]+\/)?(.*)/);
			m[1] = (m[1] ? m[1].replace(/\./g, "\\.").replace(/\?/g, "[^\\/]?").replace(/\*/g, "[^\\/]*") : "");
			m[2] = (m[2] ? m[2].replace(/\./g, "\\.").replace(/\+/g, "\\+").replace(/\?/g, "\\?").replace(/\*/g, ".*") : "");
			pattern = m[1] + m[2];
			pattern = "^" + pattern.replace(/\/$/, "\/.*") + "$";
		}
		var reg = new RegExp(pattern.toLowerCase());
		return (reg.test(url.toLowerCase()));
	},
	isMatchFilterList: function(url) {
		var aList = this.getPrefFilterList();
		for (var i=0; i<aList.length; i++) {
			var item = aList[i].split("\b");
			var rule = item[0];
			var enabled = (item.length == 1);
			if (enabled && this.isMatchURL(url, rule)) return(true);
		}
		return(false);
	},
	getTopWinBrowser: function() {
		try {
			var winMgr = Cc['@mozilla.org/appshell/window-mediator;1'].getService();
			var topWin = winMgr.QueryInterface(Ci.nsIWindowMediator).getMostRecentWindow("navigator:browser");
			var mBrowser = topWin.document.getElementById("content");
			return mBrowser;
		} catch(e) {}
		return null;
	},
	autoSwitchFilter: function(url) {
		if (url == "about:blank") return;
		var mBrowser = this.getTopWinBrowser();
		if (!(mBrowser && mBrowser.mIeTabSwitchURL)) return;
		if (mBrowser.mIeTabSwitchURL == url) {
			var isMatched = false;
			var aList = this.getPrefFilterList();
			var isIE = this.isIeTabURL(url);
			if (isIE) url = decodeURI(url.substring(this.IETAB_CHROME_URL.length));
			for (var i=0; i<aList.length; i++) {
				var item = aList[i].split("\b");
				var rule = item[0];
				if (this.isMatchURL(url, rule)) {
					aList[i] = rule + (isIE ? "" : "\b");
					isMatched = true;
				}
			}
			if (isMatched) this.setPrefFilterList(aList);
		}
		mBrowser.mIeTabSwitchURL = null;
	}
}

function IeTabContentPolicy() { }
IeTabContentPolicy.prototype = {
	classDescription: "IE Tab URL Filter",
	classID: Components.ID("{3fdaa104-5988-4050-94fc-c711d568fe64}"),
	contractID: "@mozilla.org/ietabfilter;1",
	QueryInterface: XPCOMUtils.generateQI([Ci.nsIContentPolicy, Ci.nsISupportsWeakReference, Ci.nsISupports]),
	shouldFilter: function(url) {
		return !IeTabFilter.isIeTabURL(url)
			 && IeTabFilter.isFilterEnabled()
			 && IeTabFilter.isMatchFilterList(url);
	},
	shouldLoad: function(aContentType, aContentLocation, aRequestOrigin, aRequestingNode, aMimeTypeGuess, aExtra) {
		if (aContentType == Ci.nsIContentPolicy.TYPE_DOCUMENT) {
			IeTabFilter.autoSwitchFilter(aContentLocation.spec);
			if (this.shouldFilter(aContentLocation.spec)) {
				aContentLocation.spec = IeTabFilter.getIeTabURL(aContentLocation.spec);
			}
		}
		return Ci.nsIContentPolicy.ACCEPT;
	},
	shouldProcess: function(aContentType, aContentLocation, aRequestOrigin, aRequestingNode, aMimeType, aExtra) {
		return Ci.nsIContentPolicy.ACCEPT;
	}
};

const NSGetFactory = XPCOMUtils.generateNSGetFactory([IeTabContentPolicy]);
