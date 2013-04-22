//
// ietabSettings.js
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

function IeTab() {}

IeTab.prototype = {
   IExploreExePath: ""
}

IeTab.prototype.getPrefFilterList = function() {
   var s = this.getStrPref("ietab.filterlist", null);
   return (s ? s.split(" ") : "");
}

IeTab.prototype.addFilterRule = function(rule, enabled) {
   var rules = document.getElementById('filterChilds');
   var item = document.createElement('treeitem');
   var row = document.createElement('treerow');
   var c1 = document.createElement('treecell');
   var c2 = document.createElement('treecell');
   c1.setAttribute('label', rule);
   c2.setAttribute('value', enabled);
   row.appendChild(c1);
   row.appendChild(c2);
   item.appendChild(row);
   rules.appendChild(item);
   return (rules.childNodes.length-1);
}

IeTab.prototype.initDialog = function() {
   //get iexplore.exe path
   this.IExploreExePath = IeTabExtApp.getIExploreExePath();

   //filter
   document.getElementById('filtercbx').checked = this.getBoolPref("ietab.filter", true);
   var list = this.getPrefFilterList();
   var rules = document.getElementById('filterChilds');
   while (rules.hasChildNodes()) rules.removeChild(rules.firstChild);
   for (var i = 0; i < list.length; i++) {
      if (list[i] != "") {
         var item = list[i].split("\b");
         var rule = item[0];
         if (!/^\/(.*)\/$/.exec(rule)) rule = rule.replace(/\/$/, "/*");
         var enabled = (item.length == 1);
         this.addFilterRule(rule, enabled);
      }
   }
   //general
   document.getElementById('toolsmenu').checked = this.getBoolPref("ietab.toolsmenu", true);
   document.getElementById('toolsmenu.icon').checked = this.getBoolPref("ietab.toolsmenu.icon", false);
   document.getElementById('statusbar').checked = this.getBoolPref("ietab.statusbar", true);
   document.getElementById('handleurl').checked = this.getBoolPref("ietab.handleUrlBar", false);
   document.getElementById('alwaysnew').checked = this.getBoolPref("ietab.alwaysNewTab", false);
   document.getElementById('focustab').checked  = this.getBoolPref("ietab.focustab", true);

   //context
   document.getElementById('pagelink.embed').checked = this.getBoolPref("ietab.pagelink", true);
   document.getElementById('tabsmenu.embed').checked = this.getBoolPref("ietab.tabsmenu", true);
   document.getElementById('bookmark.embed').checked = this.getBoolPref("ietab.bookmark", true);

   document.getElementById('pagelink.extapp').checked = this.getBoolPref("ietab.pagelink.extapp", true);
   document.getElementById('tabsmenu.extapp').checked = this.getBoolPref("ietab.tabsmenu.extapp", true);
   document.getElementById('bookmark.extapp').checked = this.getBoolPref("ietab.bookmark.extapp", true);

   document.getElementById('pagelink.icon').checked = this.getBoolPref("ietab.icon.pagelink", false);
   document.getElementById('tabsmenu.icon').checked = this.getBoolPref("ietab.icon.tabsmenu", false);
   document.getElementById('bookmark.icon').checked = this.getBoolPref("ietab.icon.bookmark", false);

   //external
   var path = this.getStrPref("ietab.extAppPath", "");
   document.getElementById('pathbox').value = (path == "" ? this.IExploreExePath : path);
   document.getElementById('parambox').value = this.getStrPref("ietab.extAppParam", "%1");
   document.getElementById('ctrlclick').checked = this.getBoolPref("ietab.ctrlclick", true);

   //fill urlbox
   var newurl = (window.arguments ? window.arguments[0] : ""); //get CurrentTab's URL
   document.getElementById('urlbox').value = ( this.startsWith(newurl,"about:") ? "" : newurl);
   document.getElementById('urlbox').select();

   //updateStatus
   this.updateDialogPositions();
   this.updateDialogAllStatus();
   this.updateApplyButton(false);
}

IeTab.prototype.updateApplyButton = function(e) {
   document.getElementById("myExtra1").disabled = !e;
}

IeTab.prototype.init = function() {
   this.initDialog();
   this.addEventListenerByTagName("checkbox", "command", this.updateApplyButton);
   this.addEventListener("filterChilds", "DOMAttrModified", this.updateApplyButton);
   this.addEventListener("filterChilds", "DOMNodeInserted", this.updateApplyButton);
   this.addEventListener("filterChilds", "DOMNodeRemoved", this.updateApplyButton);
   this.addEventListener("parambox", "input", this.updateApplyButton);
   this.addEventListener("toolsmenu", "command", this.updateToolsMenuStatus);
}

IeTab.prototype.destory = function() {
   this.removeEventListenerByTagName("checkbox", "command", this.updateApplyButton);
   this.removeEventListener("filterChilds", "DOMAttrModified", this.updateApplyButton);
   this.removeEventListener("filterChilds", "DOMNodeInserted", this.updateApplyButton);
   this.removeEventListener("filterChilds", "DOMNodeRemoved", this.updateApplyButton);
   this.removeEventListener("parambox", "input", this.updateApplyButton);
   this.removeEventListener("toolsmenu", "command", this.updateToolsMenuStatus);
}

IeTab.prototype.updateInterface = function() {
   var statusbar = this.getBoolPref("ietab.statusbar", true);
   var icon = (window.arguments ? window.arguments[1] : null); //get status-bar icon handle
   if (icon) this.setAttributeHidden(icon, statusbar);
}

IeTab.prototype.setOptions = function() {
   //filter
   var filter = document.getElementById('filtercbx').checked;
   this.setBoolPref("ietab.filter", filter);
   this.setStrPref("ietab.filterlist", this.getFilterListString());

   //general
   var toolsmenu = document.getElementById('toolsmenu').checked;
   this.setBoolPref("ietab.toolsmenu", toolsmenu);
   this.setBoolPref("ietab.toolsmenu.icon", document.getElementById('toolsmenu.icon').checked);

   var statusbar = document.getElementById('statusbar').checked;
   this.setBoolPref("ietab.statusbar", statusbar);

   this.setBoolPref("ietab.handleUrlBar", document.getElementById('handleurl').checked);
   this.setBoolPref("ietab.alwaysNewTab", document.getElementById('alwaysnew').checked);
   this.setBoolPref("ietab.focustab", document.getElementById('focustab').checked);

   //context (ietab)
   this.setBoolPref("ietab.pagelink", document.getElementById('pagelink.embed').checked);
   this.setBoolPref("ietab.tabsmenu", document.getElementById('tabsmenu.embed').checked);
   this.setBoolPref("ietab.bookmark", document.getElementById('bookmark.embed').checked);

   //context (extapp)
   this.setBoolPref("ietab.pagelink.extapp", document.getElementById('pagelink.extapp').checked);
   this.setBoolPref("ietab.tabsmenu.extapp", document.getElementById('tabsmenu.extapp').checked);
   this.setBoolPref("ietab.bookmark.extapp", document.getElementById('bookmark.extapp').checked);

   //showicon
   this.setBoolPref("ietab.icon.pagelink", document.getElementById('pagelink.icon').checked);
   this.setBoolPref("ietab.icon.tabsmenu", document.getElementById('tabsmenu.icon').checked);
   this.setBoolPref("ietab.icon.bookmark", document.getElementById('bookmark.icon').checked);

   //external
   var path = document.getElementById('pathbox').value;
   this.setStrPref("ietab.extAppPath", (path == this.IExploreExePath ? "" : path));

   var param = document.getElementById('parambox').value;
   this.setStrPref("ietab.extAppParam", this.trim(param).split(/\s+/).join(" "));

   this.setBoolPref("ietab.ctrlclick", document.getElementById('ctrlclick').checked);

   //update UI
   this.updateApplyButton(false);
   this.updateInterface();
}

IeTab.prototype.setAttributeHidden = function(obj, isHidden) {
   if (!obj) return;
   if (isHidden){
      obj.removeAttribute("hidden");
   }else{
      obj.setAttribute("hidden", true);
   }
}

IeTab.prototype.getFilterListString = function() {
   var list = [];
   var filter = document.getElementById('filterList');
   var count = filter.view.rowCount;

   for (var i=0; i<count; i++) {
      var rule = filter.view.getCellText(i, filter.columns['columnRule']);
      var enabled = filter.view.getCellValue(i, filter.columns['columnEnabled']);
      var item = rule + (enabled=="true" ? "" : "\b");
      list.push(item);
   }
   list.sort();
   return list.join(" ");
}

IeTab.prototype.updateToolsMenuStatus = function() {
   document.getElementById("toolsmenu.icon").disabled = !document.getElementById("toolsmenu").checked;
}

IeTab.prototype.updateDelButtonStatus = function() {
   var en = document.getElementById('filtercbx').checked;
   var delbtn = document.getElementById('delbtn');
   var filter = document.getElementById('filterList');
   delbtn.disabled = (!en) || (filter.view.selection.count < 1);
}

IeTab.prototype.updateAddButtonStatus = function() {
   var en = document.getElementById('filtercbx').checked;
   var addbtn = document.getElementById('addbtn');
   var urlbox = document.getElementById('urlbox');
   addbtn.disabled = (!en) || (this.trim(urlbox.value).length < 1);
}

IeTab.prototype.updateDialogAllStatus = function() {
   var en = document.getElementById('filtercbx').checked;
   document.getElementById('filterList').disabled = (!en);
   document.getElementById('filterList').editable = (en);
   document.getElementById('urllabel').disabled = (!en);
   document.getElementById('urlbox').disabled = (!en);
   this.updateAddButtonStatus();
   this.updateDelButtonStatus();
   this.updateToolsMenuStatus();
}

IeTab.prototype.updateDialogPositions = function() {
   var em = [document.getElementById('tabsmenu.embed'),
             document.getElementById('pagelink.embed'),
             document.getElementById('bookmark.embed')]
   var ex = [document.getElementById('tabsmenu.extapp'),
             document.getElementById('pagelink.extapp'),
             document.getElementById('bookmark.extapp')]
   var emMax = Math.max(em[0].boxObject.width, em[1].boxObject.width, em[2].boxObject.width);
   var exMax = Math.max(ex[0].boxObject.width, ex[1].boxObject.width, ex[2].boxObject.width);
   for (var i=0 ; i<em.length ; i++) em[i].width = emMax;
   for (var i=0 ; i<ex.length ; i++) ex[i].width = exMax;
}

IeTab.prototype.findRule = function(value) {
   var filter = document.getElementById('filterList');
   var count = filter.view.rowCount;
   for (var i=0; i<count; i++) {
      var rule = filter.view.getCellText(i, filter.columns['columnRule']);
      if (rule == value) return i;
   }
   return -1;
}

IeTab.prototype.addNewURL = function() {
   var filter = document.getElementById('filterList');
   var urlbox = document.getElementById('urlbox');
   var rule = this.trim(urlbox.value);
   if (rule != "") {
      if ((rule != "about:blank") && (rule.indexOf("://") < 0)) {
         rule = (/^[A-Za-z]:/.test(rule) ? "file:///"+rule.replace(/\\/g,"/") : rule);
         if (/^file:\/\/.*/.test(rule)) rule = encodeURI(rule);
      }
      if (!/^\/(.*)\/$/.exec(rule)) rule = rule.replace(/\/$/, "/*");
      rule = rule.replace(/\s/g, "%20");
      var idx = this.findRule(rule);
      if (idx == -1) { idx = this.addFilterRule(rule, true); urlbox.value = ""; }
      filter.view.selection.select(idx);
      filter.boxObject.ensureRowIsVisible(idx);
   }
   filter.focus();
   this.updateAddButtonStatus();
}

IeTab.prototype.delSelected = function() {
   var filter = document.getElementById('filterList');
   var rules = document.getElementById('filterChilds');
   if (filter.view.selection.count > 0) {
      for (var i=rules.childNodes.length-1 ; i>=0 ; i--) {
         if (filter.view.selection.isSelected(i))
            rules.removeChild(rules.childNodes[i]);
      }
   }
   this.updateDelButtonStatus();
}

IeTab.prototype.onClickFilterList = function(e) {
   var filter = document.getElementById('filterList');
   if (!filter.disabled && e.button == 0 && e.detail >= 2) {
      if (filter.view.selection.count == 1) {
         var urlbox = document.getElementById('urlbox');
         urlbox.value = filter.view.getCellText(filter.currentIndex, filter.columns['columnRule']);
         urlbox.select();
         this.updateAddButtonStatus();
      }
   }
}

IeTab.prototype.modifyTextBoxValue = function(textboxId, newValue) {
   var box = document.getElementById(textboxId);
   if (box.value != newValue) {
      box.value = newValue;
      this.updateApplyButton(true);
   }
}

IeTab.prototype.browseAppPath = function() {
   const nsIFilePicker = Components.interfaces.nsIFilePicker;
   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
   fp.init(window, null, nsIFilePicker.modeOpen);
   fp.appendFilters(nsIFilePicker.filterApps);
   fp.appendFilters(nsIFilePicker.filterAll);
   var rv = fp.show();
   if (rv == nsIFilePicker.returnOK) {
      this.modifyTextBoxValue("pathbox", fp.file.target);
   }
}

IeTab.prototype.resetAppPath = function() {
   this.modifyTextBoxValue("pathbox", this.IExploreExePath);
   this.modifyTextBoxValue("parambox", "%1");
}

IeTab.prototype.saveToFile = function(aList) {
   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
   var stream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
   var converter = Components.classes["@mozilla.org/intl/converter-output-stream;1"].createInstance(Components.interfaces.nsIConverterOutputStream);

   fp.init(window, null, fp.modeSave);
   fp.defaultExtension = "txt";
   fp.defaultString = "IETabPref";
   fp.appendFilters(fp.filterText);

   if (fp.show() != fp.returnCancel) {
      try {
         if (fp.file.exists()) fp.file.remove(true);
         fp.file.create(fp.file.NORMAL_FILE_TYPE, 0666);
         stream.init(fp.file, 0x02, 0x200, null);
         converter.init(stream, "UTF-8", 0, 0x0000);

         for (var i = 0; i < aList.length ; i++) {
            aList[i] = aList[i] + "\n";
            converter.writeString(aList[i]);
         }
      } finally {
         converter.close();
         stream.close();
      }
   }
}

IeTab.prototype.loadFromFile = function() {
   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
   var stream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
   var converter = Components.classes["@mozilla.org/intl/converter-input-stream;1"].createInstance(Components.interfaces.nsIConverterInputStream);

   fp.init(window, null, fp.modeOpen);
   fp.defaultExtension = "txt";
   fp.appendFilters(fp.filterText);

   if (fp.show() != fp.returnCancel) {
      try {
         var input = {};
         stream.init(fp.file, 0x01, 0444, null);
         converter.init(stream, "UTF-8", 0, 0x0000);
         converter.readString(stream.available(), input);
         var linebreak = input.value.match(/(((\n+)|(\r+))+)/m)[1];
         return input.value.split(linebreak);
      } finally {
         converter.close();
         stream.close();
      }
   }
   return null;
}

IeTab.prototype.getAllSettings = function(isDefault) {
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = (isDefault ? prefservice.getDefaultBranch("extensions.") : prefservice.getBranch("extensions.") );
   var preflist = prefs.getChildList("ietab.", {});

   var aList = ["IETabPref"];
   for (var i = 0 ; i < preflist.length ; i++) {
      try {
         var value = null;
         switch (prefs.getPrefType(preflist[i])) {
         case prefs.PREF_BOOL:
            value = prefs.getBoolPref(preflist[i]);
            break;
         case prefs.PREF_INT:
            value = prefs.getIntPref(preflist[i]);
            break;
         case prefs.PREF_STRING:
            value = prefs.getComplexValue(preflist[i], Components.interfaces.nsISupportsString).data;
            break;
         }
         aList.push(preflist[i] + "=" + value);
      } catch (e) {}
   }
   return aList;
}

IeTab.prototype.setAllSettings = function(aList) {
   if (!aList) return;
   if (aList.length == 0) return;
   if (aList[0] != "IETabPref") return;

   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("extensions.");

   var aPrefs = [];
   for (var i = 1 ; i < aList.length ; i++){
      var index = aList[i].indexOf("=");
      if (index > 0){
         var name = aList[i].substring(0, index);
         var value = aList[i].substring(index+1, aList[i].length);
         if (this.startsWith(name, "ietab.")) aPrefs.push([name, value]);
      }
   }
   for (var i = 0 ; i < aPrefs.length ; i++) {
      try {
         var name = aPrefs[i][0];
         var value = aPrefs[i][1];
         switch (prefs.getPrefType(name)) {
         case prefs.PREF_BOOL:
            prefs.setBoolPref(name, /true/i.test(value));
            break;
         case prefs.PREF_INT:
            prefs.setIntPref(name, value);
            break;
         case prefs.PREF_STRING:
            if (value.indexOf('"') == 0) value = value.substring(1, value.length-1);
            var sString = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
            sString.data = value;
            prefs.setComplexValue(name, Components.interfaces.nsISupportsString, sString);
            break;
         }
      } catch (e) {}
   }
}

IeTab.prototype.exportSettings = function() {
   var aList = this.getAllSettings();
   if (aList) this.saveToFile(aList);
}

IeTab.prototype.importSettings = function() {
   var aList = this.loadFromFile();
   if (aList) {
      this.setAllSettings(aList);
      this.initDialog();
      this.updateInterface();
   }
}

IeTab.prototype.restoreDefault = function() {
   var aTemp = this.getAllSettings(false);
   var aDefault = this.getAllSettings(true);
   this.setAllSettings(aDefault);
   this.initDialog();
   this.setAllSettings(aTemp);
   this.updateApplyButton(true);
}

var gIeTab = new IeTab();
