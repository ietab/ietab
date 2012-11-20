#pragma once

#include <inttypes.h>
#include <stdint.h>

#include "nptypes.h"
#include "npapi.h"
#include "npruntime.h"

class CNPVariant : public NPVariant {
public:

	CNPVariant(void) {
		VOID_TO_NPVARIANT(*this);
	}

	CNPVariant(int value) {
		INT32_TO_NPVARIANT(value, *this);
	}

	CNPVariant(bool value) {
		BOOLEAN_TO_NPVARIANT(value, *this);
	}

	CNPVariant(char* value) {
		STRINGZ_TO_NPVARIANT(value, *this);
	}

	CNPVariant(NPObject* value) {
		OBJECT_TO_NPVARIANT(value, *this);
	}

	CNPVariant(double value) {
		DOUBLE_TO_NPVARIANT(value, *this);
	}

	~CNPVariant(void) {
		NPN_ReleaseVariantValue(this);
	}

	CNPVariant& operator=(int value) {
		INT32_TO_NPVARIANT(value, *this);
		return *this;
	}

	CNPVariant& operator=(bool value) {
		BOOLEAN_TO_NPVARIANT(value, *this);
		return *this;
	}

	CNPVariant& operator=(char* value) {
		STRINGZ_TO_NPVARIANT(value, *this);
		return *this;
	}

	CNPVariant& operator=(NPObject* value) {
		OBJECT_TO_NPVARIANT(value, *this);
		return *this;
	}

	CNPVariant& operator=(double value) {
		DOUBLE_TO_NPVARIANT(value, *this);
		return *this;
	}

	operator int() {
		return NPVARIANT_TO_INT32(*this);
	}

	operator bool() {
		return NPVARIANT_TO_BOOLEAN(*this);
	}

	operator NPString() {
		return NPVARIANT_TO_STRING(*this);
	}

	operator const char*() {
		NPString str = NPVARIANT_TO_STRING(*this);
		return str.UTF8Characters;
	}

	operator NPObject*() {
		return NPVARIANT_TO_OBJECT(*this);
	}

	operator double() {
		return NPVARIANT_TO_DOUBLE(*this);
	}
};

