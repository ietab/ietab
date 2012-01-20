#pragma once

#include <inttypes.h>
#include <stdint.h>

#include "nptypes.h"
#include "npapi.h"
#include "npruntime.h"

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>

// Helper class that can be used to map calls to the NPObject hooks
// into virtual methods on instances of classes that derive from this
// class.
class ScriptablePluginObjectBase : public NPObject
{
public:
    ScriptablePluginObjectBase(NPP npp)
        : mNpp(npp)
    {
    }

    virtual ~ScriptablePluginObjectBase()
    {
    }

    // Virtual NPObject hooks called through this base class. Override
    // as you see fit.
    virtual void Invalidate();
    virtual bool HasMethod(NPIdentifier name);
    virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                        uint32_t argCount, NPVariant *result);
    virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                               NPVariant *result);
    virtual bool HasProperty(NPIdentifier name);
    virtual bool GetProperty(NPIdentifier name, NPVariant *result);
    virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
    virtual bool RemoveProperty(NPIdentifier name);
    virtual bool Enumerate(NPIdentifier **identifier, uint32_t *count);
    virtual bool Construct(const NPVariant *args, uint32_t argCount,
                           NPVariant *result);

public:
    static void _Deallocate(NPObject *npobj);
    static void _Invalidate(NPObject *npobj);
    static bool _HasMethod(NPObject *npobj, NPIdentifier name);
    static bool _Invoke(NPObject *npobj, NPIdentifier name,
                        const NPVariant *args, uint32_t argCount,
                        NPVariant *result);
    static bool _InvokeDefault(NPObject *npobj, const NPVariant *args,
                               uint32_t argCount, NPVariant *result);
    static bool _HasProperty(NPObject * npobj, NPIdentifier name);
    static bool _GetProperty(NPObject *npobj, NPIdentifier name,
                             NPVariant *result);
    static bool _SetProperty(NPObject *npobj, NPIdentifier name,
                             const NPVariant *value);
    static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);
    static bool _Enumerate(NPObject *npobj, NPIdentifier **identifier,
                           uint32_t *count);
    static bool _Construct(NPObject *npobj, const NPVariant *args,
                           uint32_t argCount, NPVariant *result);

protected:
    NPP mNpp;
};


class CPlugin;

class ScriptablePluginObject : public ScriptablePluginObjectBase
{
public:

	static NPObject* New(NPP npp);

	// Do not use new to create scriptable object.
	// Call ScriptablePluginObject::New() instead.!
    ScriptablePluginObject(NPP npp)
        : ScriptablePluginObjectBase(npp)
    {
		static bool symbolsInitialized = false;
		if(!symbolsInitialized) {
			InitSymbols();
			symbolsInitialized = true;
		}
    }

    virtual ~ScriptablePluginObject() {
    }

    inline CPlugin* GetPlugin() {
        return reinterpret_cast<CPlugin*>(mNpp->pdata);
    }
	static void InitSymbols();

	// NPObject functions
    virtual bool HasMethod(NPIdentifier name);
    virtual bool HasProperty(NPIdentifier name);
    virtual bool GetProperty(NPIdentifier name, NPVariant *result);
	virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
    virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                        uint32_t argCount, NPVariant *result);
    virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                               NPVariant *result);

private:
    CPlugin* plugin;
};


