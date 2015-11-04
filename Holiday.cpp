
#include "Platform.h"
#include "Holiday.h"
#include <mutex>
#include "Strict.h"

#include "HolidayData.h"
#include "Algorithms.h"

static std::mutex TheHolidayComboMutex;
#define LOCK_COMBOS std::lock_guard<std::mutex> l(TheHolidayComboMutex)

namespace
{
	bool operator<(const Handle_<HolidayCenterData_>& lhs, const Handle_<HolidayCenterData_>& rhs) { return lhs->center_ < rhs->center_; }
	// call this function only after calling Unique() on the parts
	String_ NameFromCenters(const Vector_<Handle_<HolidayCenterData_>>& parts)
	{
		static const auto ToName = [](const Handle_<HolidayCenterData_> h) {return h->center_; };
		return String::Accumulate(Apply(ToName, parts), " ");
	}
	// index this map ONLY using canonical center-set strings
	std::map<String_, Handle_<HolidayCenterData_>>& TheCombinations()
	{
      RETURN_STATIC(std::map<String_, Handle_<HolidayCenterData_>>);
	}
}

Holidays_::Holidays_(const String_& src)
{
	// split on space
	Vector_<String_> centers = String::Split(src, ' ', false);
	parts_ = Unique(Apply([](const String_& c){return Holidays::OfCenter(Holidays::CenterIndex(c)); }, centers));
	if (parts_.size() > 1)
	{
		// check for an existing combined data, but do not create one
		LOCK_COMBOS;
		auto existing = TheCombinations().find(NameFromCenters(parts_));
		if (existing != TheCombinations().end())
			parts_ = Vector::V1(existing->second);
	}
}

bool Holidays_::IsHoliday(const Date_& date) const
{
	for (const auto& ps : parts_)
		if (BinarySearch(ps->holidays_, date))
			return true;
	return false;
}

String_ Holidays_::String() const
{
	return NameFromCenters(parts_);
}

CountBusDays_::CountBusDays_(const Holidays_& src) : hols_(src)
{
	if (src.parts_.size() > 1)	// the holiday calendar is not merged
	{
		LOCK_COMBOS;
		Handle_<HolidayCenterData_>& combo = TheCombinations()[src.String()];
		if (combo.Empty())
		{
			Vector_<Date_> merged;
			for (const auto& p : src.parts_)
				merged.Append(p->holidays_);
			combo.reset(new HolidayCenterData_(src.String(), Unique(merged)));
		}
		// change our internal instance (Holidays_ constructed after this will also be merged)
		hols_.parts_ = Vector::V1(combo);
	}
}

int CountBusDays_::operator()(const Date_& start, const Date_& end) const
{
	if (end <= start)
		return 0;
	const int weeks = (end - start) / 7;
	const Date_& stop = start.AddDays(7 * weeks);
	auto pStop = LowerBound(hols_.parts_[0]->holidays_, stop);
	const Vector_<Date_>& hols = hols_.parts_[0]->holidays_;
	const auto holsToStop = static_cast<int>(pStop - LowerBound(hols, start));	// # which are >= start and < stop
	int retval = 5 * weeks - holsToStop;
	// now add the fraction-of-week
	for (Date_ d = stop; d < end; ++d)
		if (!Date::IsWeekend(d))
		{
			if (*pStop == d)
				++pStop;
			else
				++retval;
		}

	return retval;
}

Date_ Holidays::NextBus(const Holidays_& hols, const Date_& from)
{
	for (Date_ retval = from;; ++retval)
	{
		if (!Date::IsWeekend(retval) && !hols.IsHoliday(retval))
			return retval;
	}
}
Date_ Holidays::PrevBus(const Holidays_& hols, const Date_& from)
{
	for (Date_ retval = from;; --retval)
	{
      if (!Date::IsWeekend(retval) && !hols.IsHoliday(retval))
			return retval;
	}
}

const Holidays_& Holidays::None()
{
	static const Holidays_ RETVAL("");
	return RETVAL;
}