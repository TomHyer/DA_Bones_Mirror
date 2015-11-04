
#include "Platform.h"
#include "DateTimeUtils.h"
#include "Strict.h"

#include "Algorithms.h"
#include "DateTime.h"
#include "DateUtils.h"
#include "Exceptions.h"

bool DateTime::IsDateTimeString(const String_& src)
{
	// I fear this is going to be a performance bottleneck
	auto space = src.find(' ');
	if (space == String_::npos)
		return false;
	if (!Date::IsDateString(src.substr(0, space)))
		return false;
	// accept anything with a date, space, something, colon, something
	auto colon = src.find(':', space);
	return colon != String_::npos
			&& colon > space + 1
			&& colon + 1 < src.size();
}

DateTime_ DateTime::FromString(const String_& src)
{
	NOTICE(src);
	auto space = src.find(' ');
	const Date_ date = Date::FromString(src.substr(0, space));
	if (space == String_::npos)
		return DateTime_(date, 0);
	// split remainder on ':'
	Vector_<String_> tParts = String::Split(src.substr(space + 1), ':', true);
	REQUIRE(tParts.size() >= 2 && tParts.size() <= 3, "Expected hh:mm or hh:mm:ss time");
	Vector_<int> hms = Apply(String::ToInt, tParts);
	REQUIRE(*MinElement(hms) >= 0 && hms[0] < 24 && *MaxElement(hms) < 60, "Hour/minute/second out of bounds");
	return DateTime_(date, hms[0], hms[1], hms.size() > 2 ? hms[2] : 0);
}

