
#include "Platform.h"
#include "Default.h"
#include "Strict.h"

#include "Exceptions.h"
#include "Algorithms.h"

AssignCreditId_::AssignCreditId_(const Vector_<String_>& names)
:
names_(Unique(names))
{  }

CreditId_ AssignCreditId_::operator()(const String_& nm)
{
	auto pn = LowerBound(names_, nm);
	REQUIRE(pn != names_.end(), "Reference credit '" + nm + "' is not part of the trade underlying");
	return CreditId_(static_cast<int>(pn - names_.begin()));
}

