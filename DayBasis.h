
// Daycount basis

#pragma once

#include <set>
#include "Vectors.h"
#include "Date.h"
#include "Strings.h"

class Date_;

/*IF--------------------------------------------------------------------------
enumeration DayBasis
	Daycounts for accrued interest
extensible
alternative ACT_365F ACT/365F ACT_365FIXED ACT/365FIXED
	Always uses a 365-day year
alternative ACT_365L ACT/365L ISMA_Year
	Sometimes uses a 366-day year
alternative ACT_360 ACT/360 MONEY ACTUAL/360
alternative ACT_ACT ACT/ACT ACTUAL/ACTUAL
alternative BOND 30_360 30/360 30_360_US
method double operator()(const Date_& start_date, const Date_& end_date, const DayBasis::Context_* context) const;
-IF-------------------------------------------------------------------------*/

namespace DayBasis
{
	// extra information which can influence the daycount for some methods
	struct Context_
	{
		bool isLast_;
		Date_ nominalStart_;
		Date_ nominalEnd_;
		int couponMonths_;
	};
}

#include "MG_DayBasis_enum.h"

