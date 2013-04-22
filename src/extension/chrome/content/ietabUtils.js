//-----------------------------------------------------------------------------

IeTab.prototype.mlog = function(text) {
  Components.classes["@mozilla.org/consoleservice;1"]
    .getService(Components.interfaces.nsIConsoleService)
    .logStringMessage("IeTab: "+text);
}

//-----------------------------------------------------------------------------

IeTab.prototype.addEventListener = function(obj, type, listener) {
   if (typeof(obj) == "string") obj = document.getElementById(obj);
   if (obj) obj.addEventListener(type, listener, false);
}
IeTab.prototype.removeEventListener = function(obj, type, listener) {
   if (typeof(obj) == "string") obj = document.getElementById(obj);
   if (obj) obj.removeEventListener(type, listener, false);
}

IeTab.prototype.addEventListenerByTagName = function(tag, type, listener) {
   var objs = document.getElementsByTagName(tag);
   for (var i = 0; i < objs.length; i++) {
      objs[i].addEventListener(type, listener, false);
   }
}
IeTab.prototype.removeEventListenerByTagName = function(tag, type, listener) {
   var objs = document.getElementsByTagName(tag);
   for (var i = 0; i < objs.length; i++) {
      objs[i].removeEventListener(type, listener, false);
   }
}

//-----------------------------------------------------------------------------

IeTab.prototype.hookCode = function(orgFunc, orgCode, myCode) {
   try{ if (eval(orgFunc).toString() == eval(orgFunc + "=" + eval(orgFunc).toString().replace(orgCode, myCode))) throw orgFunc; }
   catch(e){ Components.utils.reportError("Failed to hook function: "+orgFunc); }
}

IeTab.prototype.hookAttr = function(parentNode, attrName, myFunc) {
   if (typeof(parentNode) == "string") parentNode = document.getElementById(parentNode);
   try { parentNode.setAttribute(attrName, myFunc + parentNode.getAttribute(attrName)); }catch(e){ Components.utils.reportError("Failed to hook attribute: "+attrName); }
}

IeTab.prototype.hookProp = function(parentNode, propName, myGetter, mySetter) {
   var oGetter = parentNode.__lookupGetter__(propName);
   var oSetter = parentNode.__lookupSetter__(propName);
   if (oGetter && myGetter) myGetter = oGetter.toString().replace(/{/, "{"+myGetter.toString().replace(/^.*{/,"").replace(/.*}$/,""));
   if (oSetter && mySetter) mySetter = oSetter.toString().replace(/{/, "{"+mySetter.toString().replace(/^.*{/,"").replace(/.*}$/,""));
   if (!myGetter) myGetter = oGetter;
   if (!mySetter) mySetter = oSetter;
   if (myGetter) try { eval('parentNode.__defineGetter__(propName, '+ myGetter.toString() +');'); }catch(e){ Components.utils.reportError("Failed to hook property Getter: "+propName); }
   if (mySetter) try { eval('parentNode.__defineSetter__(propName, '+ mySetter.toString() +');'); }catch(e){ Components.utils.reportError("Failed to hook property Setter: "+propName); }
}

//-----------------------------------------------------------------------------

IeTab.prototype.trim = function(s) {
   if (s) return s.replace(/^\s+/g,"").replace(/\s+$/g,""); else return "";
}

IeTab.prototype.startsWith = function(s, prefix) {
   if (s) return( (s.substring(0, prefix.length) == prefix) ); else return false;
}

//-----------------------------------------------------------------------------

IeTab.prototype.getBoolPref = function(prefName, defval) {
   var result = defval;
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("extensions.");
   if (prefs.getPrefType(prefName) == prefs.PREF_BOOL) {
       try { result = prefs.getBoolPref(prefName); }catch(e){}
   }
   return(result);
}

IeTab.prototype.getIntPref = function(prefName, defval) {
   var result = defval;
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("extensions.");
   if (prefs.getPrefType(prefName) == prefs.PREF_INT) {
       try { result = prefs.getIntPref(prefName); }catch(e){}
   }
   return(result);
}

IeTab.prototype.getStrPref = function(prefName, defval) {
   var result = defval;
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("extensions.");
   if (prefs.getPrefType(prefName) == prefs.PREF_STRING) {
       try { result = prefs.getComplexValue(prefName, Components.interfaces.nsISupportsString).data; }catch(e){}
   }
   return(result);
}

//-----------------------------------------------------------------------------

IeTab.prototype.setBoolPref = function(prefName, value) {
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("extensions.");
   try { prefs.setBoolPref(prefName, value); } catch(e){}
}

IeTab.prototype.setIntPref = function(prefName, value) {
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("extensions.");
   try { prefs.setIntPref(prefName, value); } catch(e){}
}

IeTab.prototype.setStrPref = function(prefName, value) {
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("extensions.");
   var sString = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
   sString.data = value;
   try { prefs.setComplexValue(prefName, Components.interfaces.nsISupportsString, sString); } catch(e){}
}

//-----------------------------------------------------------------------------

IeTab.prototype.getDefaultCharset = function(defval) {
   var charset = this.getStrPref("ietab.intl.charset.default", "");
   if (charset.length) return charset;
	var gPrefs = Components.classes['@mozilla.org/preferences-service;1'].getService(Components.interfaces.nsIPrefBranch);
	if(gPrefs.prefHasUserValue("intl.charset.default")) {
	   return gPrefs.getCharPref("intl.charset.default");
	} else {
	   var strBundle = Components.classes["@mozilla.org/intl/stringbundle;1"].getService(Components.interfaces.nsIStringBundleService);
	   var intlMess = strBundle.createBundle("chrome://global-platform/locale/intl.properties");
	   try {
	      return intlMess.GetStringFromName("intl.charset.default");
	   } catch(e) {
   	   return defval;
      }
	}
}

IeTab.prototype.convertToUTF8 = function(data, charset) {
   try {
      data = decodeURI(data);
   }catch(e){
      if (!charset) charset = gIeTab.getDefaultCharset();
      if (charset) {
         var uc = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
         try {
            uc.charset = charset;
            data = uc.ConvertToUnicode(unescape(data));
            data = decodeURI(data);
         }catch(e){}
         uc.Finish();
      }
   }
   return data;
}

IeTab.prototype.convertToASCII = function(data, charset) {
   if (!charset) charset = gIeTab.getDefaultCharset();
   if (charset) {
      var uc = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
      uc.charset = charset;
      try {
         data = uc.ConvertFromUnicode(data);
      }catch(e){
         data = uc.ConvertToUnicode(unescape(data));
         data = decodeURI(data);
         data = uc.ConvertFromUnicode(data);
      }
      uc.Finish();
   }
   return data;
}

//-----------------------------------------------------------------------------

IeTab.prototype.getUrlDomain = function(url) {
   if (url && !gIeTab.startsWith(url, "about:")) {
      if (/^file:\/\/.*/.test(url)) return url;
      var matches = url.match(/^([A-Za-z]+:\/+)*([^\:^\/]+):?(\d*)(\/.*)*/);
      if (matches) url = matches[1]+matches[2]+(matches[3]==""?"":":"+matches[3])+"/";
   }
   return url;
}

IeTab.prototype.getUrlHost = function(url) {
   if (url && !gIeTab.startsWith(url, "about:")) {
      if (/^file:\/\/.*/.test(url)) return url;
      var matches = url.match(/^([A-Za-z]+:\/+)*([^\:^\/]+):?(\d*)(\/.*)*/);
      if (matches) url = matches[2];
   }
   return url;
}

//-----------------------------------------------------------------------------
