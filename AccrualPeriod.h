
// unit from which legs are composed

#pragma once

#include "Date.h"
#include "DayBasis.h"

struct AccrualPeriod_
{
	Date_ startDate_;
	Date_ endDate_;
	double notional_;
	DayBasis_ couponBasis_;
	double dcf_;   // redundant but handy
	Handle_<DayBasis::Context_> context_;
	bool isStub_;   // used in forming Libor rates

	~AccrualPeriod_();
	AccrualPeriod_();
	AccrualPeriod_(const Date_& start, const Date_& end, double notional, const DayBasis_& daycount);
	AccrualPeriod_(const Date_& start, const Date_& end, double notional, const DayBasis_& daycount, const Handle_<DayBasis::Context_>& context, bool is_stub);
};
