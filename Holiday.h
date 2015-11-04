
// holiday calendars for use in schedule generation

#pragma once

#include "HolidayData.h"
class String_;
class Date_;

class Holidays_     
{
	Vector_<Handle_<HolidayCenterData_>> parts_;	// calendars for each center
	friend class CountBusDays_;
public:
	Holidays_(const String_& src);
	String_ String() const;
	bool IsHoliday(const Date_& date) const;
};

namespace Holidays
{
	const Holidays_& None();
	Date_ NextBus(const Holidays_& hols, const Date_& from);	// returns input date if it is already a good business day
	Date_ PrevBus(const Holidays_& hols, const Date_& from);
}

// this is internally optimized with precomputed merged schedules
// always use it; never count business days by hand
class CountBusDays_
{
	Holidays_ hols_;
public:
	CountBusDays_(const Holidays_& holidays);
	int operator()(const Date_& begin, const Date_& end) const;
};

