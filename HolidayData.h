
// data supporting implementation of holiday calendars; see Holiday.h for interface

#pragma once

#include <map>
#include "Vectors.h"
#include "Strings.h"
#include "Date.h"

struct HolidayCenterData_
{
	String_ center_;
	Vector_<Date_> holidays_;
	HolidayCenterData_(const String_& c, const Vector_<Date_>& h) : center_(c), holidays_(h) {}
};

struct HolidayData_
{
	Vector_<Handle_<HolidayCenterData_>> holidays_;
	std::map<String_, int> centerIndex_;

	bool IsValid() const;
	void Swap(HolidayData_* other);
};

namespace Holidays
{
	void AddCenter
		(const String_& city,
		 const Vector_<Date_>& holidays);

	// lookup by int, plus separate string-to-int recognition
	// this helps separate the recognition algorithm from the canonical name
	int CenterIndex(const String_& center);
	Handle_<HolidayCenterData_> OfCenter(int center_index);
}

