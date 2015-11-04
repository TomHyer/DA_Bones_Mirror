
// a leg is a vector of periods

#pragma once

#include "AccrualPeriod.h"
#include "CouponRate.h"
#include "Date.h"

struct LegPeriod_
{
	Handle_<AccrualPeriod_> accrual_;
	Handle_<CouponRate_> rate_;
	Date_ payDate_;
};

