/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * 2011-12-25 modified by Hong Jen Yee (PCMan) <pcman.tw@gmail.com> for IE Tab.
 * This file is splitted from plugin.cpp to make code more moduler and 
 * readable.
 *
 * ***** END LICENSE BLOCK ***** */

#include "ScriptablePluginObject.h"
#include "plugin.h"

// properties
static NPIdentifier callbacks_id;
static NPIdentifier canClose_id;
static NPIdentifier canBack_id;
static NPIdentifier canForward_id;
static NPIdentifier canRefresh_id;
static NPIdentifier canStop_id;
static NPIdentifier progress_id;
static NPIdentifier security_id;
static NPIdentifier url_id;
static NPIdentifier title_id;
static NPIdentifier canCut_id;
static NPIdentifier canCopy_id;
static NPIdentifier canPaste_id;

// methods
static NPIdentifier goBack_id;
static NPIdentifier goForward_id;
static NPIdentifier navigate_id;
static NPIdentifier refresh_id;
static NPIdentifier stop_id;
static NPIdentifier saveAs_id;
static NPIdentifier print_id;
static NPIdentifier printPreview_id;
static NPIdentifier printSetup_id;
static NPIdentifier cut_id;
static NPIdentifier copy_id;
static NPIdentifier paste_id;
static NPIdentifier selectAll_id;
static NPIdentifier find_id;
static NPIdentifier viewSource_id;
static NPIdentifier focus_id;

static NPIdentifier sPluginType_id;

#define DECLARE_NPOBJECT_CLASS_WITH_BASE(_class, ctor)                        \
static NPClass s##_class##_NPClass = {                                        \
  NP_CLASS_STRUCT_VERSION_CTOR,                                               \
  ctor,                                                                       \
  ScriptablePluginObjectBase::_Deallocate,                                    \
  ScriptablePluginObjectBase::_Invalidate,                                    \
  ScriptablePluginObjectBase::_HasMethod,                                     \
  ScriptablePluginObjectBase::_Invoke,                                        \
  ScriptablePluginObjectBase::_InvokeDefault,                                 \
  ScriptablePluginObjectBase::_HasProperty,                                   \
  ScriptablePluginObjectBase::_GetProperty,                                   \
  ScriptablePluginObjectBase::_SetProperty,                                   \
  ScriptablePluginObjectBase::_RemoveProperty,                                \
  ScriptablePluginObjectBase::_Enumerate,                                     \
  ScriptablePluginObjectBase::_Construct                                      \
}

#define GET_NPOBJECT_CLASS(_class) &s##_class##_NPClass

void
ScriptablePluginObjectBase::Invalidate()
{
}

bool
ScriptablePluginObjectBase::HasMethod(NPIdentifier name)
{
    return false;
}

bool
ScriptablePluginObjectBase::Invoke(NPIdentifier name, const NPVariant *args,
                                   uint32_t argCount, NPVariant *result)
{
    return false;
}

bool
ScriptablePluginObjectBase::InvokeDefault(const NPVariant *args,
        uint32_t argCount, NPVariant *result)
{
    return false;
}

bool
ScriptablePluginObjectBase::HasProperty(NPIdentifier name)
{
    return false;
}

bool
ScriptablePluginObjectBase::GetProperty(NPIdentifier name, NPVariant *result)
{
    return false;
}

bool
ScriptablePluginObjectBase::SetProperty(NPIdentifier name,
                                        const NPVariant *value)
{

    return false;
}

bool
ScriptablePluginObjectBase::RemoveProperty(NPIdentifier name)
{
    return false;
}

bool
ScriptablePluginObjectBase::Enumerate(NPIdentifier **identifier,
                                      uint32_t *count)
{
    return false;
}

bool
ScriptablePluginObjectBase::Construct(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
    return false;
}

// static
void
ScriptablePluginObjectBase::_Deallocate(NPObject *npobj)
{
    // Call the virtual destructor.
    delete (ScriptablePluginObjectBase *)npobj;
}

// static
void
ScriptablePluginObjectBase::_Invalidate(NPObject *npobj)
{
    ((ScriptablePluginObjectBase *)npobj)->Invalidate();
}

// static
bool
ScriptablePluginObjectBase::_HasMethod(NPObject *npobj, NPIdentifier name)
{
    return ((ScriptablePluginObjectBase *)npobj)->HasMethod(name);
}

// static
bool
ScriptablePluginObjectBase::_Invoke(NPObject *npobj, NPIdentifier name,
                                    const NPVariant *args, uint32_t argCount,
                                    NPVariant *result)
{
    return ((ScriptablePluginObjectBase *)npobj)->Invoke(name, args, argCount,
            result);
}

// static
bool
ScriptablePluginObjectBase::_InvokeDefault(NPObject *npobj,
        const NPVariant *args,
        uint32_t argCount,
        NPVariant *result)
{
    return ((ScriptablePluginObjectBase *)npobj)->InvokeDefault(args, argCount,
            result);
}

// static
bool
ScriptablePluginObjectBase::_HasProperty(NPObject * npobj, NPIdentifier name)
{
    return ((ScriptablePluginObjectBase *)npobj)->HasProperty(name);
}

// static
bool
ScriptablePluginObjectBase::_GetProperty(NPObject *npobj, NPIdentifier name,
        NPVariant *result)
{
    return ((ScriptablePluginObjectBase *)npobj)->GetProperty(name, result);
}

// static
bool
ScriptablePluginObjectBase::_SetProperty(NPObject *npobj, NPIdentifier name,
        const NPVariant *value)
{
    return ((ScriptablePluginObjectBase *)npobj)->SetProperty(name, value);
}

// static
bool
ScriptablePluginObjectBase::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
    return ((ScriptablePluginObjectBase *)npobj)->RemoveProperty(name);
}

// static
bool
ScriptablePluginObjectBase::_Enumerate(NPObject *npobj,
                                       NPIdentifier **identifier,
                                       uint32_t *count)
{
    return ((ScriptablePluginObjectBase *)npobj)->Enumerate(identifier, count);
}

// static
bool
ScriptablePluginObjectBase::_Construct(NPObject *npobj, const NPVariant *args,
                                       uint32_t argCount, NPVariant *result)
{
    return ((ScriptablePluginObjectBase *)npobj)->Construct(args, argCount,
            result);
}


class ConstructablePluginObject : public ScriptablePluginObjectBase
{
public:
    ConstructablePluginObject(NPP npp)
        : ScriptablePluginObjectBase(npp)
    {
    }

    virtual bool Construct(const NPVariant *args, uint32_t argCount,
                           NPVariant *result);
};

static NPObject *
AllocateConstructablePluginObject(NPP npp, NPClass *aClass)
{
    // MessageBox(0, L"AllocateConstructablePluginObject", 0, 0);
    return new ConstructablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ConstructablePluginObject,
                                 AllocateConstructablePluginObject);

bool
ConstructablePluginObject::Construct(const NPVariant *args, uint32_t argCount,
                                     NPVariant *result)
{
    NPObject *myobj =
        NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(ConstructablePluginObject));
    if (!myobj)
        return false;

    OBJECT_TO_NPVARIANT(myobj, *result);

    return true;
}


static NPObject *
AllocateScriptablePluginObject(NPP npp, NPClass *aClass)
{
    return new ScriptablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject,
                                 AllocateScriptablePluginObject);

NPObject* ScriptablePluginObject::New(NPP npp) {
	return NPN_CreateObject(npp, GET_NPOBJECT_CLASS(ScriptablePluginObject));
}

bool
ScriptablePluginObject::HasMethod(NPIdentifier name)
{
    return (name == goBack_id ||
            name == goForward_id ||
            name == navigate_id ||
            name == refresh_id ||
            name == stop_id ||
            name == saveAs_id ||
            name == print_id ||
            name == printPreview_id ||
            name == printSetup_id ||
            name == cut_id ||
            name == copy_id ||
            name == paste_id ||
            name == selectAll_id ||
            name == find_id ||
            name == viewSource_id ||
            name == focus_id);
}


bool
ScriptablePluginObject::HasProperty(NPIdentifier name)
{
    return (name == callbacks_id ||
            name == canClose_id ||
            name == canBack_id ||
            name == canForward_id ||
            name == canRefresh_id ||
            name == canStop_id ||
            name == canCut_id ||
            name == canCopy_id ||
            name == canPaste_id ||
            name == progress_id ||
            name == security_id ||
            name == url_id ||
            name == title_id||
            name == sPluginType_id);
}

bool
ScriptablePluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
    VOID_TO_NPVARIANT(*result);

    if(name == callbacks_id) {
		NPObject* target = GetPlugin()->GetCallbacks();
		if(target)
			NPN_RetainObject(target);
		OBJECT_TO_NPVARIANT(target, *result);
    }
    else if(name == canClose_id) {
		BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanClose(), *result);
    }
    else if(name == canBack_id) {
		BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanBack(), *result);
    }
    else if(name == canForward_id) {
		BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanForward(), *result);
    }
    else if(name == canRefresh_id) {
		BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanRefresh(), *result);
    }
    else if(name == canStop_id) {
		BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanStop(), *result);
    }
    else if(name == canCut_id) {
        BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanCut(), *result);
    }
    else if(name == canCopy_id) {
		BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanCopy(), *result);
    }
    else if(name == canPaste_id) {
		BOOLEAN_TO_NPVARIANT(GetPlugin()->GetCanPaste(), *result);
    }
    else if(name == progress_id) {
		INT32_TO_NPVARIANT(GetPlugin()->GetProgress(), *result);
    }
    else if(name == security_id) {
		INT32_TO_NPVARIANT(GetPlugin()->GetSecurity(), *result);
    }
    else if(name == url_id) {
		char* url = GetPlugin()->GetUrl();
        STRINGZ_TO_NPVARIANT(url, *result);
    }
    else if(name == title_id) {
		char* title = GetPlugin()->GetTitle();
        STRINGZ_TO_NPVARIANT(title, *result);
    }
    else if (name == sPluginType_id) {
        // FIXME: seriously, what on earth is this for?
        NPObject *myobj =
            NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(ConstructablePluginObject));
        if (!myobj) {
            return false;
        }
        OBJECT_TO_NPVARIANT(myobj, *result);
    }
    else {
        return false;
    }
    return true;
}

bool
ScriptablePluginObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
    if(name == callbacks_id) {
		NPObject* target = NPVARIANT_TO_OBJECT(*value);
		GetPlugin()->SetCallbacks(target);
    }
    else if(name == url_id) {
		const NPString url = NPVARIANT_TO_STRING(*value);
		GetPlugin()->Navigate((const char*)url.UTF8Characters);
    }
	else {
	    return false;
	}
	return true;
}

bool
ScriptablePluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
	CPlugin* plugin = GetPlugin();
    if(name == goBack_id) {
		plugin->GoBack();
    }
    else if(name == goForward_id) {
		plugin->GoForward();
    }
    else if(name == navigate_id) {
		if(argCount >= 1) {
			if(NPVARIANT_IS_STRING(args[0])) {
				plugin->Navigate(NPVARIANT_TO_STRING(args[0]).UTF8Characters);
				BOOLEAN_TO_NPVARIANT(true, *result);
			}
		}
		return true;
    }
    else if(name == refresh_id) {
		plugin->Refresh();
    }
    else if(name == stop_id) {
		plugin->Stop();
    }
    else if(name == saveAs_id) {
		plugin->SaveAs();
    }
    else if(name == print_id) {
		plugin->Print();
    }
    else if(name == printPreview_id) {
		plugin->PrintPreview();
	}
    else if(name == printSetup_id) {
		plugin->PrintSetup();
    }
    else if(name == cut_id) {
		plugin->Cut();
    }
    else if(name == copy_id) {
		plugin->Copy();
    }
    else if(name == paste_id) {
		plugin->Paste();
    }
    else if(name == selectAll_id) {
		plugin->SelectAll();
    }
    else if(name == find_id) {
		plugin->Find();
    }
    else if(name == viewSource_id) {
		plugin->ViewSource();
    }
    else if(name == focus_id) {
		plugin->Focus();
    }
    else {
        return false;
    }
    return true;
}

bool
ScriptablePluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
    printf ("ScriptablePluginObject default method called!\n");

    STRINGZ_TO_NPVARIANT(strdup("default method return val"), *result);

    return true;
}


void ScriptablePluginObject::InitSymbols() {
    // properties
    callbacks_id = NPN_GetStringIdentifier("callbacks");
    canClose_id = NPN_GetStringIdentifier("canClose");
    canBack_id = NPN_GetStringIdentifier("canBack");
    canForward_id = NPN_GetStringIdentifier("canForward");
    canRefresh_id = NPN_GetStringIdentifier("canRefresh");
    canStop_id = NPN_GetStringIdentifier("canStop");
    canCut_id = NPN_GetStringIdentifier("canCut");
    canCopy_id = NPN_GetStringIdentifier("canCopy");
    canPaste_id = NPN_GetStringIdentifier("canPaste");
    progress_id = NPN_GetStringIdentifier("progress");
    security_id = NPN_GetStringIdentifier("security");
    url_id = NPN_GetStringIdentifier("url");
    title_id = NPN_GetStringIdentifier("title");

    // methods
    goBack_id = NPN_GetStringIdentifier("goBack");
    goForward_id = NPN_GetStringIdentifier("goForward");
    navigate_id = NPN_GetStringIdentifier("navigate");
    refresh_id = NPN_GetStringIdentifier("refresh");
    stop_id = NPN_GetStringIdentifier("stop");
    saveAs_id = NPN_GetStringIdentifier("saveAs");
    print_id = NPN_GetStringIdentifier("print");
    printPreview_id = NPN_GetStringIdentifier("printPreview");
    printSetup_id = NPN_GetStringIdentifier("printSetup");
    cut_id = NPN_GetStringIdentifier("cut");
    copy_id = NPN_GetStringIdentifier("copy");
    paste_id = NPN_GetStringIdentifier("paste");
    selectAll_id = NPN_GetStringIdentifier("selectAll");
    find_id = NPN_GetStringIdentifier("find");
    viewSource_id = NPN_GetStringIdentifier("viewSource");
    focus_id = NPN_GetStringIdentifier("focus");

    sPluginType_id = NPN_GetStringIdentifier("PluginType");
}
