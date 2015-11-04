
// higher-level DateTime functionality
	// e.g., functions that might throw

#pragma once

class String_;
class DateTime_;

namespace DateTime
{
	bool IsDateTimeString(const String_& src);
	DateTime_ FromString(const String_& src);	// date, space, colon-separated numbers
}

