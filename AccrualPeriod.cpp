
#include "Platform.h"
#include "AccrualPeriod.h"
#include "Strict.h"

AccrualPeriod_::~AccrualPeriod_() {}
AccrualPeriod_::AccrualPeriod_() {}	// uninitialized

AccrualPeriod_::AccrualPeriod_
	(const Date_& start,
	 const Date_& end,
	 double notional, 
	 const DayBasis_& daycount)
:
startDate_(start),
endDate_(end),
notional_(notional),
couponBasis_(daycount),
dcf_(daycount(start, end, nullptr)),
isStub_(false)
{	}

AccrualPeriod_::AccrualPeriod_
	(const Date_& start,
	 const Date_& end, 
	 double notional,
	 const DayBasis_& daycount,
	 const Handle_<DayBasis::Context_>& context,
	 bool is_stub)
:
startDate_(start),
endDate_(end),
notional_(notional),
couponBasis_(daycount),
dcf_(daycount(start, end, context.get())),
context_(context),
isStub_(is_stub)
{	}

