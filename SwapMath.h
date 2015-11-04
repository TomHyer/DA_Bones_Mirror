
// fast calculators for swap cashflow rollout, sacrificing polymorphism
	// this is where swap confirm definitions are embodied

#pragma once

#include "DateTime.h"
struct Cell_;
class Ccy_;

namespace SwapMath
{
	struct FixedPeriod_
	{
		Date_ accrueFrom_;
		Date_ accrueTo_;
		Date_ payDate_;
	};

	Vector_<FixedPeriod_> FixedLegDates
		(const Ccy_& ccy,
		 const Date_& start,
		 const Cell_& maturity);


	struct LiborPeriod_
	{
		Date_ accrueFrom_;
		Date_ accrueTo_;
		DateTime_ fixDate_;
		Date_ payDate_;
	};

	Vector_<LiborPeriod_> LiborLegDates
		(const Ccy_& ccy,
		 const Date_& start,
		 const Cell_& maturity);

}