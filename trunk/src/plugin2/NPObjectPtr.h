#pragma once

#include <inttypes.h>
#include <stdint.h>

#include "nptypes.h"
#include "npapi.h"
#include "npruntime.h"
#include <cstdarg>

// An easy to use C++ wrapper for NPObject

class CNPObjectPtr
{
public:
	CNPObjectPtr(NPObject* object = NULL) : p(object) {
		if(object)
			NPN_RetainObject(object);
	}

	~CNPObjectPtr(void) {
		if(p)
			NPN_ReleaseObject(p);
			p = NULL;
	}

	CNPObjectPtr(CNPObjectPtr& another) {
		if(p)
			NPN_ReleaseObject(p);
		p = another.p;
		if(p)
			NPN_RetainObject(p);
	}

	CNPObjectPtr& operator=(NPObject* object) {
		if(p)
			NPN_ReleaseObject(p);
		p = object;
		if(p)
			NPN_RetainObject(p);
		return *this;
	}

	operator NPObject*() {
		return p;
	}

	bool Invoke(NPP npp, NPIdentifier method, void* retval, const char* format, ...) {
		va_list ap;
		va_start(ap, format);
		bool ret = InvokeV(npp, method, retval, format, ap);
		va_end(ap);
		return ret;
	}

	bool Invoke(NPP npp, const char* method_name, void* retval, const char* format, ...) {
		NPIdentifier method = NPN_GetStringIdentifier(method_name);
		va_list ap;
		va_start(ap, format);
		bool ret = InvokeV(npp, method, retval, format, ap);
		va_end(ap);
		return ret;
	}

	bool InvokeV(NPP npp, NPIdentifier method, void* retval, const char* format, va_list ap);

	bool GetChildProperty(NPP npp, NPVariant* ret, ... /*NPIdentifier child_element, ...*/);

	bool GetChildPropertyV(NPP npp, NPVariant* ret, va_list child_ids);

	bool GetProperty(NPP npp, NPVariant* ret, NPIdentifier property) {
		return NPN_GetProperty(npp, p, property, ret);
	}

	bool GetProperty(NPP npp, NPVariant* ret, char* property) {
		NPIdentifier property_id = NPN_GetStringIdentifier(property);
		return NPN_GetProperty(npp, p, property_id, ret);
	}


	NPObject* p;
};
