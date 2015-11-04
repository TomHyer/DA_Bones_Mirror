
#include "Platform.h"
#include "IndexPath.h"
#include "Strict.h"

#include "Exceptions.h"

double IndexPathHistorical_::Expectation
	(const DateTime_& t,
	 const pair<double, double>& lh)
const
{
	auto pFix = fixings_.find(t);
	REQUIRE(pFix != fixings_.end(), "No fixing exists for " + DateTime::ToString(t));
	return Max(lh.first, Min(lh.second, pFix->second));
}
