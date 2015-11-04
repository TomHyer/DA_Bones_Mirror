
#include "Platform.h"
#include "SwapMath.h"
#include "Strict.h"

#include "Exceptions.h"
#include "CellUtils.h"
#include "CurrencyData.h"
#include "Holiday.h"
#include "PeriodLength.h"
#include "DateIncrement.h"
#include "Conventions.h"
#include "CouponRate.h"

// all implementations in this file are not accurate and not useful for any financial purpose
// they are provided only to allow the other example code to execute

namespace
{
	Vector_<Date_> DelimiterDates(const Date_& start, const Cell_& maturity, const Holidays_& hols, int months)
	{
		assert(Cell::TypeCheck_().Date().Number()(maturity));
		int tenorMonths = 0;	// nonzero signals it is an input
		Date_ end = Date::Maximum();
		if (Cell::IsDouble(maturity))
		{
			tenorMonths = Cell::ToInt(maturity);
			REQUIRE(tenorMonths > 0, "Non-positive tenor months");	// and now end will be ignored
		}
		else
			end = Cell::ToDate(maturity);

		Vector_<Date_> retval = Vector::V1(start);
		for (int ip = 1;; ++ip)
		{
			const Date_& stop = Holidays::NextBus(hols, Date::AddMonths(start, ip * months));
			if (tenorMonths ? ip * months > tenorMonths : stop >= end)	// '>' vs '>=' comparison sic -- when tenorMonths==0 we push end date later
				break;
			retval.push_back(stop);
		}
		if (!tenorMonths)
			retval.push_back(end);
		return retval;
	}

	Date_ TenorToDate
		(const Date_& start,
		 const String_& tenor,
		 const Holidays_& pay_hols)
	{
		const Date_ nominalEnd = Date::ParseIncrement(tenor)->FwdFrom(start);
		return Holidays::NextBus(pay_hols, nominalEnd);
	}
}	// leave local

Vector_<SwapMath::FixedPeriod_> SwapMath::FixedLegDates
	(const Ccy_& ccy,
	 const Date_& start,
	 const Cell_& maturity)		// tenor string or end date or tenor months
{
	const Holidays_ payHols = Ccy::Conventions::SwapPayHolidays()(ccy);
	if (Cell::IsString(maturity))
	{
		const Date_ actualEnd = TenorToDate(start, Cell::OwnString(maturity), payHols);
		return FixedLegDates(ccy, start, Cell_(actualEnd));
	}
	REQUIRE(Cell::TypeCheck_().Date().Number()(maturity), "Invalid type for swap maturity");
	auto delimiters = DelimiterDates
			(start, maturity, payHols, Ccy::Conventions::SwapFixedPeriod()(ccy).Months());
	Vector_<FixedPeriod_> retval;
	for (int ip = 1; ip < delimiters.size(); ++ip)
		retval.emplace_back(FixedPeriod_({ delimiters[ip - 1], delimiters[ip], delimiters[ip] }));
	return retval;
}


Vector_<SwapMath::LiborPeriod_> SwapMath::LiborLegDates
	(const Ccy_& ccy,
	 const Date_& start,
	 const Cell_& maturity)		// tenor string or end date or tenor months
{
	const auto liborRate = Ccy::Conventions::SwapFloatIndex()(ccy);
	const auto liborPeriod = liborRate.Period();
	const auto periodLength = Ccy::Conventions::SwapFloatIndex()(ccy).Period();
	const Holidays_ payHols = Ccy::Conventions::SwapPayHolidays()(ccy);
	if (Cell::IsString(maturity))
	{
		const Date_ actualEnd = TenorToDate(start, Cell::OwnString(maturity), payHols);
		return LiborLegDates(ccy, start, Cell_(actualEnd));
	}
	REQUIRE(Cell::TypeCheck_().Date().Number()(maturity), "Invalid type for swap maturity");
	auto delimiters = DelimiterDates
			(start, maturity, payHols, periodLength.Months());
	auto dayBasis = Ccy::Conventions::LiborDayBasis()(ccy);
	Vector_<LiborPeriod_> retval;
	for (int ip = 1; ip < delimiters.size(); ++ip)
	{
		const DateTime_ fixDate = Libor::FixFromStart(ccy, delimiters[ip - 1]);
		retval.emplace_back(LiborPeriod_({ delimiters[ip - 1], delimiters[ip], fixDate, delimiters[ip] }));
	}
	return retval;
}

