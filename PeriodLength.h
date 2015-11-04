
// use period length, not frequency

#pragma once

#include "Strings.h"

class Date_;
class Ccy_;

/*IF--------------------------------------------------------------------------
enumeration PeriodLength
	Standardized time intervals
alternative ANNUAL 12M 
alternative SEMIANNUAL SEMI 6M
alternative QUARTERLY 3M
alternative MONTHLY 1M
method int Months() const;
-IF-------------------------------------------------------------------------*/
#include "MG_PeriodLength_enum.h"

namespace Date
{
	Date_ NominalMaturity
		(const Date_& start,
		 const PeriodLength_& step,
		 const Ccy_& ccy);
}

