
#include "Platform.h"
#include "Fixings.h"
#include "Strict.h"

#include "Algorithms.h"
#include "Exceptions.h"

const FixHistory_& FixHistory::Empty()
{
	static const FixHistory_ RETVAL((FixHistory_::vals_t()));
	return RETVAL;
}

double FixHistory_::Find(const DateTime_& fix_time, bool quiet) const
{
	struct Less_
	{
		bool operator()(const pair<DateTime_, double>& lhs, const DateTime_& rhs) const { return lhs.first < rhs; }
	};
	auto pGE = std::lower_bound(vals_.begin(), vals_.end(), fix_time, Less_());
	if (pGE == vals_.end() || pGE->first != fix_time)
	{
		REQUIRE(quiet, "No fixing for that time");
		return -DA::INFINITY;
	}
	return pGE->second;
}

