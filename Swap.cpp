
#include "Platform.h"
#include "Swap.h"
#include "Strict.h"

#include "Cell.h"
#include "Flow.h"
#include "Exceptions.h"
#include "Archive.h"
#include "ReceivePay.h"
#include "DateIncrement.h"
#include "Holiday.h"
#include "Trade.h"
#include "SwapMath.h"
#include "LegParams.h"
#include "LegSchedule.h"
#include "PeriodLength.h"
#include "Period.h"
#include "CurrencyData.h"
#include "Conventions.h"
#include "CashTrade.h"
#include "IndexIr.h"

Vector_<Handle_<LegPeriod_>> Swap::FixedSide
	(const Ccy_& ccy,
	 const Date_& start,
	 const Cell_& maturity,		// tenor string or end date
	 double notional,
	 double coupon)
{
	const auto dates = SwapMath::FixedLegDates(ccy, start, maturity);
	const auto dayBasis = Ccy::Conventions::SwapFixedDayBasis()(ccy);
	Vector_<Handle_<LegPeriod_>> retval;
	for (const auto& fp : dates)
	{
		std::unique_ptr<LegPeriod_> period(new LegPeriod_);
		period->payDate_ = fp.payDate_;
		period->accrual_.reset(new AccrualPeriod_(fp.accrueFrom_, fp.accrueTo_, notional, dayBasis));
		period->rate_.reset(new FixedRate_(coupon));
		retval.push_back(period.release());
	}
	return retval;
}

Vector_<Handle_<LegPeriod_>> Swap::FloatSide
	(const Ccy_& ccy,
	 const Date_& start,
	 const Cell_& maturity,		// tenor string or end date
	 double notional)
{
	const auto dates = SwapMath::LiborLegDates(ccy, start, maturity);
	const auto liborRate = Ccy::Conventions::SwapFloatIndex()(ccy);
	const auto dayBasis = Ccy::Conventions::LiborDayBasis()(ccy);
	Vector_<Handle_<LegPeriod_>> retval;
	for (const auto& fp : dates)
	{
		std::unique_ptr<LegPeriod_> period(new LegPeriod_);
		period->payDate_ = fp.payDate_;
		period->accrual_.reset(new AccrualPeriod_(fp.accrueFrom_, fp.accrueTo_, notional, dayBasis));
		period->rate_.reset(new LiborRate_(fp.fixDate_, ccy, liborRate));
		retval.push_back(period.release());
	}
	return retval;
}

