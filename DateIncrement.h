
// generalized increment of dates; by tenor or to special days

#pragma once

#include "Date.h"
class String_;
class Holidays_;

namespace Date
{
	class Increment_ : noncopyable
	{
	public:
		virtual ~Increment_();
		virtual Date_ FwdFrom(const Date_& date) const = 0;
		virtual Date_ BackFrom(const Date_& date) const = 0;
	};

	Handle_<Increment_> ParseIncrement(const String_& src);
   // some special-purpose increments
   Handle_<Increment_> NBusDays(int n, const Holidays_& hols);
   Handle_<Increment_> ToIMM(bool monthly);
}

inline Date_ operator+(const Date_& date, const Date::Increment_& inc)
{
	return inc.FwdFrom(date);
}
inline Date_ operator-(const Date_& date, const Date::Increment_& inc)
{
	return inc.BackFrom(date);
}

bool IsLiborTenor(const String_& tenor);
bool IsSwapTenor(const String_& tenor);

