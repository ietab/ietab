/*
 * Copyright (c) 2005 yuoo2k <yuoo2k@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

const _IETABWATCH_CID = Components.ID('{3fdaa104-5988-4050-94fc-c711d568fe64}');
const _IETABWATCH_CONTRACTID = "@mozilla.org/ietabwatch;1";
const gIeTabChromeStr = "chrome://ietab/content/reloaded.html?url=";

// IeTabWatcher object
var IeTabWatcher = {
   isIeTabURL: function(url) {
      if (!url) return false;
      return (url.indexOf(gIeTabChromeStr) == 0);
   },

   getIeTabURL: function(url) {
      if (this.isIeTabURL(url)) return url;
      if (/^file:\/\/.*/.test(url)) try { url = decodeURI(url).substring(8).replace(/\//g, "\\"); }catch(e){}
      return gIeTabChromeStr + encodeURI(url);
   },

   getBoolPref: function(prefName, defval) {
      var result = defval;
      var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
      var prefs = prefservice.getBranch("");
      if (prefs.getPrefType(prefName) == prefs.PREF_BOOL) {
          try { result = prefs.getBoolPref(prefName); }catch(e){}
      }
      return(result);
   },

   getStrPref: function(prefName, defval) {
      var result = defval;
      var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
      var prefs = prefservice.getBranch("");
      if (prefs.getPrefType(prefName) == prefs.PREF_STRING) {
          try { result = prefs.getComplexValue(prefName, Components.interfaces.nsISupportsString).data; }catch(e){}
      }
      return(result);
   },

   setStrPref: function(prefName, value) {
      var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
      var prefs = prefservice.getBranch("");
      var sString = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
      sString.data = value;
      try { prefs.setComplexValue(prefName, Components.interfaces.nsISupportsString, sString); } catch(e){}
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
         var winMgr = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService();
         var topWin = winMgr.QueryInterface(Components.interfaces.nsIWindowMediator).getMostRecentWindow("navigator:browser");
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
         if (isIE) url = decodeURI(url.substring(gIeTabChromeStr.length));
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

// ContentPolicy class
var IeTabWatchFactoryClass = {
  shouldFilter: function(url) {
    return !IeTabWatcher.isIeTabURL(url)
         && IeTabWatcher.isFilterEnabled()
         && IeTabWatcher.isMatchFilterList(url);
  },
  // nsIContentPolicy interface implementation
  shouldLoad: function(contentType, contentLocation, requestOrigin, requestingNode, mimeTypeGuess, extra) {
    if (contentType == Components.interfaces.nsIContentPolicy.TYPE_DOCUMENT) {
      IeTabWatcher.autoSwitchFilter(contentLocation.spec);
      // check IeTab FilterList
      if (this.shouldFilter(contentLocation.spec)) {
        contentLocation.spec = IeTabWatcher.getIeTabURL(contentLocation.spec);
      }
    }
    return (Components.interfaces.nsIContentPolicy.ACCEPT);
  },
  // this is now for urls that directly load media, and meta-refreshes (before activation)
  shouldProcess: function(contentType, contentLocation, requestOrigin, requestingNode, mimeType, extra) {
    return (Components.interfaces.nsIContentPolicy.ACCEPT);
  },

  get wrappedJSObject() {
    return this;
  }
}

// Factory object
var IeTabWatchFactoryFactory = {
  createInstance: function(outer, iid) {
    if (outer != null) throw Components.results.NS_ERROR_NO_AGGREGATION;
    return IeTabWatchFactoryClass;
  }
}

// Module object
var IeTabWatchFactoryModule = {
  registerSelf: function(compMgr, fileSpec, location, type) {
    compMgr = compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
    compMgr.registerFactoryLocation(_IETABWATCH_CID, "IETab content policy", _IETABWATCH_CONTRACTID, fileSpec, location, type);
    var catman = Components.classes["@mozilla.org/categorymanager;1"].getService(Components.interfaces.nsICategoryManager);
    catman.addCategoryEntry("content-policy", _IETABWATCH_CONTRACTID, _IETABWATCH_CONTRACTID, true, true);
  },

  unregisterSelf: function(compMgr, fileSpec, location) {
    compMgr = compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
    compMgr.unregisterFactoryLocation(_IETABWATCH_CID, fileSpec);
    var catman = Components.classes["@mozilla.org/categorymanager;1"].getService(Components.interfaces.nsICategoryManager);
    catman.deleteCategoryEntry("content-policy", _IETABWATCH_CONTRACTID, true);
  },

  getClassObject: function(compMgr, cid, iid) {
    if (!cid.equals(_IETABWATCH_CID))
      throw Components.results.NS_ERROR_NO_INTERFACE;

    if (!iid.equals(Components.interfaces.nsIFactory))
      throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

    return IeTabWatchFactoryFactory;
  },

  canUnload: function(compMgr) {
    return true;
  }
};

// module initialisation
function NSGetModule(comMgr, fileSpec) {
  return IeTabWatchFactoryModule;
}
