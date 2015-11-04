
#include "Platform.h"
#include "PeriodLength.h"
#include "Strict.h"

#include "Vectors.h"
#include "Date.h"
#include "Exceptions.h"

#include "MG_PeriodLength_enum.inc"

int PeriodLength_::Months() const
{
	switch (val_)
	{
	case Value_::ANNUAL:
		return 12;
	case Value_::SEMIANNUAL:
		return 6;
	case Value_::QUARTERLY:
		return 3;
	case Value_::MONTHLY:
			return 1;
	}
	assert(!"Unnatural period length");
	return 0;
}

Date_ Date::NominalMaturity
	(const Date_& start,
	 const PeriodLength_& step,
	 const Ccy_& ccy)
{
	int yy = Year(start);
	int mm = Month(start) + step.Months();
	while (mm > 12)
		++yy, mm -= 12;
	int dd = Min(Day(start), DaysInMonth(yy, mm));
	// postponed -- proper schedule math with roll convention
	return Date_(yy, mm, dd);
}

