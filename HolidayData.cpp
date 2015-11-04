
#include "Platform.h"
#include "HolidayData.h"
#include <mutex>
#include "Strict.h"

#include "Exceptions.h"
#include "Algorithms.h"

static std::mutex TheHolidayDataMutex;
#define LOCK_DATA std::lock_guard<std::mutex> l(TheHolidayDataMutex)

bool HolidayData_::IsValid() const
{
	const int nc = holidays_.size();
	if (centerIndex_.size() != nc)
		return false;
	for (const auto& c_i : centerIndex_)
	{
		if (c_i.second >= nc || c_i.second < 0)
			return false;
		if (holidays_[c_i.second]->center_ != c_i.first)
			return false;
		// could recheck monotonicity here; postponed until we can measure the time expenditure
	}
	return true;
}

void HolidayData_::Swap(HolidayData_* other)
{
	centerIndex_.swap(other->centerIndex_);
	holidays_.Swap(&other->holidays_);
}

namespace
{
	HolidayData_& TheHolidayData()
	{
      RETURN_STATIC(HolidayData_);
	}
	HolidayData_ CopyHolidayData()	// used to reduce locking in AddCenter
	{
		LOCK_DATA;
		return TheHolidayData();
	}

	bool ContainsNoWeekends(const Vector_<Date_>& dates)
	{
		for (const auto& d : dates)
			if (Date::DayOfWeek(d) % 6 == 0)	// 0=Sunday,  6=Saturday
				return false;
		return true;
	}
}

// existing centers must not be re-ordered:  Holidays_ objects store indices of their location
void Holidays::AddCenter(const String_& city,
	const Vector_<Date_>& holidays)
{
	assert(TheHolidayData().IsValid());
	assert(ContainsNoWeekends(holidays));
	assert(IsMonotonic(holidays));
   NOTICE(city);

   HolidayData_ temp(CopyHolidayData());
   REQUIRE(!temp.centerIndex_.count(city), "Duplicate holiday center");
	temp.centerIndex_[city] = temp.holidays_.size();
	temp.holidays_.push_back(std::make_shared<const HolidayCenterData_>(city, holidays));

   LOCK_DATA;
   TheHolidayData().Swap(&temp);
	assert(TheHolidayData().IsValid());
}

int Holidays::CenterIndex(const String_& center)
{
	NOTICE(center);
	LOCK_DATA;
	auto p = TheHolidayData().centerIndex_.find(center);
	REQUIRE(p != TheHolidayData().centerIndex_.end(), "Invalid holiday center");
	return p->second;
}

Handle_<HolidayCenterData_> Holidays::OfCenter(int center_index)
{
	LOCK_DATA;
	assert(center_index >= 0 && center_index < TheHolidayData().holidays_.size());
	return TheHolidayData().holidays_[center_index];
}

