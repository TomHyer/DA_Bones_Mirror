
// interface for yield curves
	// separates discounting from forecasting

#pragma once

#include "Discount.h"
#include "Currency.h"

class CollateralType_;
class PeriodLength_;

class YieldCurve_ : public Storable_
{
public:
	const Ccy_ ccy_;
	YieldCurve_(const String_& name, const String_& ccy);
	virtual const DiscountCurve_& Discount(const CollateralType_& collateral) const = 0;
	virtual double FwdLibor
		(const PeriodLength_& tenor, const Date_& fixing_date)
	const = 0;
};

