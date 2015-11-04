
#pragma once

#include "Strings.h"
template<class T_> class Vector_;

namespace File
{
	void Read
		(const String_& filename,
		Vector_<String_>* dst);
}
