
#include "Platform.h"
#include "DayBasis.h"
#include <map>
#include "Strict.h"

#include "Exceptions.h"

#include "MG_DayBasis_enum.inc"

namespace
{
	// supporting functions for lots of day bases
	double ActActISDA(const Date_& from, const Date_& to)
	{
		const short yStart = Date::Year(from);
		const Date_ nextYear(yStart + 1, 1, 1);
		const double denom = nextYear - Date_(yStart, 1, 1);
		if (to <= nextYear)	// done
			return (to - from) / denom;
		return (nextYear - from) / denom + ActActISDA(nextYear, to);
	}

	double AnnualDaysL(const Date_& from, const Date_& to)
	{
		int mm = Date::Month(from.AddDays(1)), yy = Date::Year(from.AddDays(1));
		for (;;)
		{	// step through Februaries
			if (mm > 2)
				++yy;
			mm = 2;
			if (Date_(yy, 3, 1) > to)
				return 365.0;
			if (Date::DaysInMonth(yy, 2) == 29)
				return 366.0;
		}
	}
	double DaysL(const Date_& end)
	{
		const short yy = Date::Year(end);
		return Date_(yy + 1, 1, 1) - Date_(yy, 1, 1);
	}

	double Act365L(const Date_& from, const Date_& to, bool is_annual, const Date_& end)
	{
		return (to - from) / (is_annual ? AnnualDaysL(from, end) : DaysL(end));
	}

	double Bond(const Date_& from, const Date_& to)
	{
		short y1 = Date::Year(from), m1 = Date::Month(from), d1 = Date::Day(from);
		short y2 = Date::Year(to), m2 = Date::Month(to), d2 = Date::Day(to);
		if (m1 == 2 && d1 == Date::DaysInMonth(y1, m1))
		{
			if (m2 == 2 && d2 == Date::DaysInMonth(y2, m2))
				d2 = 30;
			d1 = 30;
		}
		if (d1 > 30)
			d2 = Min(d1, d2);
		return (360 * (y2 - y1) + 30 * (m2 - m1) + (d2 - d1)) / 360.0;
	}
}

double DayBasis_::operator()(const Date_& from, const Date_& to, const DayBasis::Context_* info) const
{
	switch (val_)
	{
	default:
		assert(!"Unrecognized day basis");
	case Value_::ACT_360:
		return (to - from) / 360.0;
	case Value_::ACT_365F:
		return (to - from) / 365.0;
	case Value_::ACT_ACT:
		return ActActISDA(from, to);
	case Value_::ACT_365L:
		REQUIRE(info, "ACT/365L daycount requires nominal end date");
		return Act365L(from, to, info->couponMonths_ == 12, info->nominalEnd_);
	case Value_::BOND:
		return Bond(from, to);
	}
}

