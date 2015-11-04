
// trades made from legs 
	// implementation support is in LegBased.h

#pragma once

#include "DateTime.h"
#include "CouponRate.h"

class LegScheduleParams_;
class RecPay_;
class TradeData_;
struct LegPeriod_;
class CollateralType_;

TradeData_* NewFixedLeg
	(const String_& name,
	 const Ccy_& ccy,
	 const Handle_<LegScheduleParams_>& terms, 
	 const RecPay_& rec_pay,
	 const Vector_<>& notional,
	 const Vector_<>& coupon,
	 const CollateralType_& collateral);

// an interface for semianalytic pricer to see
struct IsLegs_
{
	virtual Vector_<LegPeriod_> Periods(const Ccy_& pay_ccy) const = 0;
};
struct LiborFlow_
{
	DateTime_ fixDate_;
	Date_ payDate_;
	double leverage_;	// notional and daycount
	TradedRate_ rate_;
};

TradeData_* NewLiborLeg
	(const String_& name,
	 const Ccy_& ccy,
	 const Handle_<LegScheduleParams_>& terms, 
	 const RecPay_& rec_pay,
	 const Vector_<>& notional,
	 const TradedRate_& libor_rate,
	 const CollateralType_& collateral);
