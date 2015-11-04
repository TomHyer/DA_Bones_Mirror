
#include "Platform.h"
#include "Conventions.h"
#include "Strict.h"

#include "DateTime.h"
#include "Holiday.h"
#include "CurrencyData.h"

Date_ Libor::StartFromFix(const Ccy_& ccy, const Date_& fix_date)
{
	const Holidays_& hols = Ccy::Conventions::LiborFixHolidays()(ccy);
	const int nDays = Ccy::Conventions::LiborFixDays()(ccy);
	if (nDays == 0)
		return Holidays::NextBus(hols, fix_date);
	// normal case -- increment, go to business day
	Date_ retval = fix_date;
	for (int ii = 0; ii < nDays; ++ii)
		retval = Holidays::NextBus(hols, retval.AddDays(1));
	return retval;
}

namespace
{	
	Date_ FixDateFromStart(const Ccy_& ccy, const Date_& start)
	{
		const Holidays_& hols = Ccy::Conventions::LiborFixHolidays()(ccy);
		const int nDays = Ccy::Conventions::LiborFixDays()(ccy);
		if (nDays == 0)
			return Holidays::PrevBus(hols, start);
		Date_ retval = start;
		for (int ii = 0; ii < nDays; ++ii)
			retval = Holidays::PrevBus(hols, retval.AddDays(-1));
		return retval;
	}
}	// leave local

// POSTPONED:  fix this to be consistent with StartFromFix, and to know correct fixing time
DateTime_ Libor::FixFromStart(const Ccy_& ccy, const Date_& start_date)
{
	return DateTime_(FixDateFromStart(ccy, start_date), 10);
}

