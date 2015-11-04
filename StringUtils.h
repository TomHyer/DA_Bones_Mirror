
// higher-level string functionality (e.g., functions which may throw exceptions)

#pragma once

#include "Strings.h"

namespace String
{
	bool ToBool(const String_& src);	// we are fairly generous, allow y/n, t/f, 1/0 as well as true/false
	Vector_<bool> ToBoolVector(const String_& src);
}

