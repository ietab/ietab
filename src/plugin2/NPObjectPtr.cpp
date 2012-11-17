//
// NPObjectPtr.cpp: smart pointer wrapping NPObject
//
// Copyright (C) 2012 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
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

#include "NPObjectPtr.h"
#include <malloc.h>
#include "NPVariant.h"

/*
Format symbols to designate argument type:
s: string (char*)
i: int32
o: object
b: boolean
d: double
v: void (used for return value only)
The first variable arg in argument list is the address of returned value
*/
bool CNPObjectPtr::InvokeV(NPP npp, NPIdentifier method, void* retval, const char* format, va_list ap) {
	if(p == NULL || format == NULL)
		return false;

	int n_args = (int)strlen(format) - 1;
	if(n_args < 0)
		return false;

	char retval_type = format[0];
	++format; // remove signature for return type

	NPVariant* args;
	if(n_args > 0) // allocate arguments on stack
		args = reinterpret_cast<NPVariant*>(_alloca(sizeof(NPVariant) * n_args));
	else
		args = NULL;

	for(int i = 0; i < n_args; ++i) {
		switch(format[i]) {
		case 'b': {
			bool arg = va_arg(ap, bool);
			BOOLEAN_TO_NPVARIANT(arg, args[i]);
			break;
			}
		case 's': {
			char* arg = va_arg(ap, char*);
			STRINGZ_TO_NPVARIANT(arg, args[i]);
			break;
			}
		case 'i': {
			int arg = va_arg(ap, int);
			INT32_TO_NPVARIANT(arg, args[i]);
			break;
			}
		case 'o': {
			NPObject* arg = va_arg(ap, NPObject*);
			OBJECT_TO_NPVARIANT(arg, args[i]);
			break;
			}
		case 'd': {
			double arg = va_arg(ap, double);
			DOUBLE_TO_NPVARIANT(arg, args[i]);
			break;
			}
		}
	}

	NPVariant result;
	NPN_Invoke(npp, p, method, args, n_args, &result);

	if(retval) { // if we need to store return value
		switch(retval_type) {
		case 'b':
			*reinterpret_cast<bool*>(retval) = NPVARIANT_TO_BOOLEAN(result);
			break;
		case 's': {
			NPString str = NPVARIANT_TO_STRING(result);
			// *reinterpret_cast<CString*>(retval) = str;
			break;
		}
		case 'i':
			*reinterpret_cast<int*>(retval) = NPVARIANT_TO_INT32(result);
			break;
		case 'o': {
			NPObject* object = NPVARIANT_TO_OBJECT(result);
			if(object)
				NPN_RetainObject(object);
			*reinterpret_cast<NPObject**>(retval) = object;
			break;
		}
		case 'd':
			*reinterpret_cast<double*>(retval) = NPVARIANT_TO_DOUBLE(result);
			break;
		}
	}

	NPN_ReleaseVariantValue(&result);

	return true;
}

bool CNPObjectPtr::GetChildProperty(NPP npp, NPVariant* ret, ... /*NPIdentifier child_element, ...*/) {
	if(p == NULL)
		return false;

	va_list child_ids;
	va_start(child_ids, ret);
	bool success = GetChildPropertyV(npp, ret, child_ids);
	va_end(child_ids);
	return success;
}

bool CNPObjectPtr::GetChildPropertyV(NPP npp, NPVariant* ret, va_list child_ids) {
	NPIdentifier child_id = va_arg(child_ids, NPIdentifier);
	if(child_id == 0)
		return true;
/*
	NPVariant value;
	if(!NPN_GetProperty(npp, p, child_id, &value))
		return false;

	if(NPVARIANT_IS_OBJECT(value)) {
		CNPObjectPtr child = NPVARIANT_TO_OBJECT(value);
		return child.GetChildPropertyV(npp, ret, child_ids);
	}
*/
	return true;
}

