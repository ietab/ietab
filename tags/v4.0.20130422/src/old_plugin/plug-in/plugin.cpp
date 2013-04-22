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
 * ***** END LICENSE BLOCK ***** */


#include "stdafx.h"

#include "npapi.h"

#include "nsCOMPtr.h"
#include "nsIDOMWindow.h"
#include "nsIDOMDocument.h"
#include "nsIDOMHTMLDocument.h"

#include "nsStringAPI.h"
#include "nsEmbedString.h"

#include <windowsx.h>

#include <wininet.h>

#include "plugin.h"
#include "ietab.h"

//////////////////////////////////////
//
// general initialization and shutdown
//
NPError NS_PluginInitialize()
{
  return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
}

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
  if(!aCreateDataStruct)
    return NULL;

  nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct);
  return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
  if(aPlugin)
    delete (nsPluginInstance *)aPlugin;
}

////////////////////////////////////////
//
// nsPluginInstance class implementation
//
nsPluginInstance::nsPluginInstance(nsPluginCreateData* aCreateDataStruct) : nsPluginInstanceBase(),
  mInstance(aCreateDataStruct->instance), mScriptablePeer(NULL), mInitialized(FALSE),
  securityCheckOK( false )
{
   mhWnd = NULL;
   m_pIETab = NULL;
   m_URL = NULL;

   if( aCreateDataStruct->mode==NP_EMBED )
   {
      for( int i=0; i < aCreateDataStruct->argc; ++i )
      {
         if( 0 == _strcmpi( "URL", aCreateDataStruct->argn[i] ) )
            m_URL = aCreateDataStruct->argv[i];
      }
   }

}

nsPluginInstance::~nsPluginInstance()
{
   // mScriptablePeer may be also held by the browser
   // so releasing it here does not guarantee that it is over
   // we should take precaution in case it will be called later
   // and zero its mPlugin member
   if( mScriptablePeer )
   {
      mScriptablePeer->setBrowser(NULL);
      NS_IF_RELEASE(mScriptablePeer);
   }
}

NPBool nsPluginInstance::init(NPWindow* aWindow)
{
  if(aWindow == NULL)
    return FALSE;

  mhWnd = (HWND)aWindow->window;
  if(mhWnd == NULL)
    return FALSE;

  SetWindowLong(mhWnd, GWL_USERDATA, (LONG)this);

  // Security check: Only URLs started with chrome:// are allowed to use this plug-in
   nsIDOMWindow* domWindow;
   NPN_GetValue( this->getInstance(), NPNVDOMWindow,
            NS_STATIC_CAST(nsIDOMWindow **, &domWindow));
   if (domWindow) {
      nsIDOMDocument* doc;
      if( NS_SUCCEEDED( domWindow->GetDocument( &doc ) ) )
      {
         nsIDOMHTMLDocument *htmlDoc;
         if( NS_SUCCEEDED( doc->QueryInterface( NS_GET_IID(nsIDOMHTMLDocument), (void**)&htmlDoc ) ) )
         {
            nsEmbedString url;
            htmlDoc->GetURL( url );
            if( 0 == wcsncmp( L"chrome:", (const wchar_t*)url.get(), 7) )
            {
               securityCheckOK = true; // This plug-in is loaded from chrome://
            }
            NS_IF_RELEASE(htmlDoc);
         }
      }

/*
//      This is not safe because nsIDOMWindowInternal is
        not an frozen interface.

//    NS_IF_ADDREF(domWindow);
      nsIDOMWindowInternal* domWindowInternal = NULL;
      domWindow->QueryInterface( NS_GET_IID(nsIDOMWindowInternal), (void**)&domWindowInternal );
      if( domWindowInternal ) {
//       NS_IF_ADDREF(domWindowInternal);
         nsIDOMLocation* location = NULL;
         if( NS_SUCCEEDED( domWindowInternal->GetLocation( &location ) )   ){
            nsEmbedString protocol;
            location->GetProtocol( protocol );
            if( 0 == wcscmp( L"chrome:", (const wchar_t*)protocol.get()) )
            {
               securityCheckOK = true; // This plug-in is loaded from chrome://
            }
            NS_IF_RELEASE(location);
         }
//       NS_IF_RELEASE(domWindowInternal);
      }
//    NS_IF_RELEASE(domWindow);
*/
   }

  if( securityCheckOK )
  {
   m_pIETab = new CIETab(mhWnd, m_URL);
   m_pIETab->init();

   SetWindowLong(mhWnd, GWL_STYLE, GetWindowLong(mhWnd, GWL_STYLE)|WS_CLIPCHILDREN);
   SetWindowLong(mhWnd, GWL_EXSTYLE, GetWindowLong(mhWnd, GWL_EXSTYLE)|WS_EX_CONTROLPARENT);
   // associate window with our nsPluginInstance object so we can access
   // it in the window procedure
  }
  mInitialized = TRUE;

  return TRUE;
}

void nsPluginInstance::shut()
{
  if( m_pIETab )
  {
    m_pIETab->destroy();
    delete m_pIETab;
  }

  mhWnd = NULL;
  mInitialized = FALSE;
}

NPBool nsPluginInstance::isInitialized()
{
  return mInitialized;
}


// ==============================
// ! Scriptability related code !
// ==============================
//
// here the plugin is asked by Mozilla to tell if it is scriptable
// we should return a valid interface id and a pointer to
// nsScriptablePeer interface which we should have implemented
// and which should be defined in the corressponding *.xpt file
// in the bin/components folder
NPError  nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
   NPError rv = NPERR_NO_ERROR;

   if (aVariable == NPPVpluginScriptableInstance) {
      nsIIETabPlugin * scriptablePeer = getScriptablePeer();
      if (scriptablePeer) {
         *(nsISupports **)aValue = scriptablePeer;
      } else
         rv = NPERR_OUT_OF_MEMORY_ERROR;
   }
   else if (aVariable == NPPVpluginScriptableIID) {
      static nsIID scriptableIID = NS_IIETABPLUGIN_IID;
      nsIID* ptr = (nsIID *)NPN_MemAlloc(sizeof(nsIID));
      if (ptr) {
         *ptr = scriptableIID;
         *(nsIID **)aValue = ptr;
      } else
         rv = NPERR_OUT_OF_MEMORY_ERROR;
   }
   return rv;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// this method will return the scriptable object (and create it if necessary)
nsScriptablePeer* nsPluginInstance::getScriptablePeer()
{
   if (!mScriptablePeer) {
      mScriptablePeer = new nsScriptablePeer();
      if(!mScriptablePeer)
         return NULL;

      NS_ADDREF(mScriptablePeer);
   }

   // add reference for the caller requesting the object
   NS_ADDREF( mScriptablePeer );
   return mScriptablePeer;
}

const char * nsPluginInstance::getVersion()
{
  return NPN_UserAgent(mInstance);
}
