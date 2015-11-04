
// Swap cashflow rollout and standard swaps

#pragma once

#include "DateTime.h"
#include "CouponRate.h"

struct Cell_;
class Ccy_;
class RecPay_;
class TradeData_;
struct LegPeriod_;
class LegScheduleParams_;
class CollateralType_;

namespace Swap
{
	Vector_<Handle_<LegPeriod_>> FixedSide
		(const Ccy_& ccy,
		 const Date_& start,
		 const Cell_& maturity,		// tenor string or end date
		 double notional,
		 double coupon);

	Vector_<Handle_<LegPeriod_>> FloatSide
		(const Ccy_& ccy,
		 const Date_& start,
		 const Cell_& maturity,	// tenor string or end date
		 double notional);
}

