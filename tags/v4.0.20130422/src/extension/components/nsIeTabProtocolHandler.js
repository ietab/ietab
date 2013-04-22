//
// npIeTabProtocolHandler.js
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

const Cc = Components.classes;
const Ci = Components.interfaces;
Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function IeTabProtocol() { }
IeTabProtocol.prototype = {
	scheme: "ie",
	classDescription: "IE Tab Protocol Handler",
	classID: Components.ID("{7f22eb55-29b0-49f1-beb8-12744891aab7}"),
	contractID: "@mozilla.org/network/protocol;1?name=ie",
	QueryInterface: XPCOMUtils.generateQI([Ci.nsIProtocolHandler, Ci.nsISupports]),
	protocolFlags: Ci.nsIProtocolHandler.URI_NORELATIVE | Ci.nsIProtocolHandler.URI_NOAUTH,
	defaultPort: -1,
	allowPort: function(aPort, aScheme) {
		return false;
	},
	newURI: function(aSpec, aOriginCharset, aBaseURI) {
		var m = /^ie:(\S+)$/.exec(aSpec);
		if (m) { aSpec = "chrome://ietab/content/ietab.xul#" + encodeURI(m[1]); }
		return Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService).newURI(aSpec, null, null);
	},
	newChannel: function(aURI) {
		return Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService).newChannel(aURI, null, null);
	}
};

const NSGetFactory = XPCOMUtils.generateNSGetFactory([IeTabProtocol]);
