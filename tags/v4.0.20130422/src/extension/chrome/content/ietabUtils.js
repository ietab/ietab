//
// ietabUtils.js
//
// Copyright (C) 2012 yuoo2k
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

/*
// We need to find a better way to implement this

IeTab.prototype.hookFunc = function(orgFunc, myFunc) {
   try{
      var oldFunc = orgFunc;
      orgFunc = function() {
         if(myFunc())
            return;
         oldFunc();
	  }
   }
   catch(e){
      Components.utils.reportError("Failed to hook function: "+orgFunc);
   }
}
*/


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

/*
orz, This does not work at all.
// http://jordanwallwork.co.uk/2013/02/intercepting-properties-with-getterssetters/
IeTab.prototype.hookProp2 = function(obj, propName, myGetter, mySetter) {
	try{
		var desc = Object.getOwnPropertyDescriptor(obj, propName);
		if(!desc) {
			var protoType = Object.getPrototypeOf(obj);
			var desc = Object.getOwnPropertyDescriptor(protoType, propName);
		}
		var oGetter = desc.get;
		var oSetter = desc.set;
		// alert(desc + "\n" + oGetter + "\n" + oSetter);
		desc = {
			configurable: true,
			enumerable: true
		};
		if(myGetter)
			desc.get = myGetter;
		else if(oGetter)
			desc.get = oGetter;

		if(mySetter)
			desc.set = mySetter;
		else if(oSetter)
			desc.set = oSetter;

		Object.defineProperty(obj, propName, desc);
		desc = Object.getOwnPropertyDescriptor(obj, propName);
		alert(desc + "\n" + desc.get + "\n" + desc.set);
	}
	catch(e) {
		alert(protoType + propName + e);
	}
}
*/

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

var keyCodeNames = new Array();
// initialize keyCodeNames
function initKeyCodeNames() {
	keyCodeNames[0x03] = "VK_CANCEL";
	keyCodeNames[0x08] = "VK_BACK";
	keyCodeNames[0x09] = "VK_TAB";
	keyCodeNames[0x0C] = "VK_CLEAR";
	keyCodeNames[0x0D] = "VK_RETURN";
	keyCodeNames[0x0D] = "VK_ENTER"
	keyCodeNames[0x10] = "VK_SHIFT"
	keyCodeNames[0x11] = "VK_CONTROL";
	keyCodeNames[0x12] = "VK_ALT";
	keyCodeNames[0x13] = "VK_PAUSE";
	keyCodeNames[0x14] = "VK_CAPS_LOCK";
	keyCodeNames[0x1B] = "VK_ESCAPE";
	keyCodeNames[0x20] = "VK_SPACE";
	keyCodeNames[0x21] = "VK_PAGE_UP";
	keyCodeNames[0x22] = "VK_PAGE_DOWN";
	keyCodeNames[0x23] = "VK_END";
	keyCodeNames[0x24] = "VK_HOME";
	keyCodeNames[0x25] = "VK_LEFT";
	keyCodeNames[0x26] = "VK_UP";
	keyCodeNames[0x27] = "VK_RIGHT";
	keyCodeNames[0x28] = "VK_DOWN";
	keyCodeNames[0x2A] = "VK_PRINTSCREEN";
	keyCodeNames[0x2D] = "VK_INSERT";
	keyCodeNames[0x2E] = "VK_DELETE";
	//keyCodeNames[] = "VK_SEMICOLON";
	//keyCodeNames[] = "VK_EQUALS";
	for(i = 0; i <=9; ++i) { // numpad 0-9
		keyCodeNames[0x60 + i] = ("VK_NUMPAD" + i);
	}
	keyCodeNames[0x6A] = "VK_MULTIPLY";
	keyCodeNames[0x6B] = "VK_ADD";
	keyCodeNames[0x6C] = "VK_SEPARATOR";
	keyCodeNames[0x6D] = "VK_SUBTRACT";
	keyCodeNames[0x6E] = "VK_DECIMAL";
	keyCodeNames[0x6F] = "VK_DIVIDE";
	for(i = 1; i <= 24; ++i) { // F1 - F24
		keyCodeNames[0x6F + i] = "VK_F" + i;
	}
	keyCodeNames[0x90] = "VK_NUM_LOCK";
	keyCodeNames[0x91] = "VK_SCROLL_LOCK";
	keyCodeNames[0xBC] = "VK_COMMA";
	keyCodeNames[0xBE] = "VK_PERIOD";
	// keyCodeNames[] = "VK_SLASH";
	// keyCodeNames[] = "VK_BACK_QUOTE";
	// keyCodeNames[] = "VK_OPEN_BRACKET";
	// keyCodeNames[] = "VK_BACK_SLASH";
	// keyCodeNames[] = "VK_CLOSE_BRACKET";
	// keyCodeNames[] = "VK_QUOTE";
	keyCodeNames[0x2F] = "VK_HELP";
}
initKeyCodeNames();

// convert virtual key code from value to string
IeTab.prototype.keyCodeToString = function(keycode_val) {
	return keyCodeNames[keycode_val];
}
