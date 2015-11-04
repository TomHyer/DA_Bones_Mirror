
#include "Platform.h"
#include "LegSchedule.h"
#include "Strict.h"

#include "Exceptions.h"
#include "Cell.h"
#include "ReceivePay.h"
#include "CurrencyData.h"
#include "Conventions.h"
#include "SwapMath.h"
#include "LegParams.h"
#include "Period.h"

Vector_<LegPeriod_> LegBuild::Fixed
	(const Ccy_& ccy,
	 const LegScheduleParams_& schedule,
	 const RecPay_& rec_pay,
	 const Vector_<>& notional,
	 const Vector_<>& coupon,
	 const NotionalExchange_* exchange)
{
	Cell_ mat = schedule.tenor_.empty() ? Cell_(schedule.matDate_) : Cell_(schedule.tenor_);
	auto dates = SwapMath::FixedLegDates(ccy, schedule.startDate_, mat);
	const int np = dates.size();
	Vector_<LegPeriod_> retval(np);
	const int deltaC = coupon.size() > 1 ? 1 : 0;	// step through coupon rates
	const int deltaN = notional.size() > 1 ? 1 : 0;	// step through coupon rates
	DayBasis_ dct = schedule.dayBasis_.empty()
			? Ccy::Conventions::SwapFixedDayBasis()(ccy)
			: DayBasis_(schedule.dayBasis_);
	for (int ip = 0; ip < np; ++ip)
	{
		retval[ip].payDate_ = dates[ip].payDate_;
		retval[ip].rate_.reset(new FixedRate_(coupon[ip * deltaC]));	// POSTPONED -- could save the memory allocation here when coupon is constant by sharing a single handle
		retval[ip].accrual_.reset(new AccrualPeriod_(dates[ip].accrueFrom_, dates[ip].accrueTo_, notional[ip * deltaN] * rec_pay.RecSign(), dct));
	}
	return retval;
}

Vector_<LegPeriod_> LegBuild::Libor
	(const Ccy_& ccy,
	 const LegScheduleParams_& schedule,
	 const RecPay_& rec_pay,
	 const Vector_<>& notional,
	 const Vector_<>& margin,
	 const Holidays_* fixing_holidays,
	 const Vector_<int>& fixing_delay,
	 const NotionalExchange_* exchange)
{
	REQUIRE(margin.empty(), "Margin on Libor is not supported");
	Cell_ mat = schedule.tenor_.empty() ? Cell_(schedule.matDate_) : Cell_(schedule.tenor_);
	auto dates = SwapMath::LiborLegDates(ccy, schedule.startDate_, mat);
	const int np = dates.size();
	Vector_<LegPeriod_> retval(np);
	const int deltaN = notional.size() > 1 ? 1 : 0;	// step through coupon rates
	DayBasis_ dct = schedule.dayBasis_.empty()
			? Ccy::Conventions::LiborDayBasis()(ccy)
			: DayBasis_(schedule.dayBasis_);
	TradedRate_ rate = Ccy::Conventions::SwapFloatIndex()(ccy);
	for (int ip = 0; ip < np; ++ip)
	{
		retval[ip].payDate_ = dates[ip].payDate_;
		auto fixDate = Libor::FixFromStart(ccy, dates[ip].accrueFrom_);
		retval[ip].rate_.reset(new LiborRate_(fixDate, ccy, rate));	// POSTPONED -- could save the memory allocation here when coupon is constant by sharing a single handle
		retval[ip].accrual_.reset(new AccrualPeriod_(dates[ip].accrueFrom_, dates[ip].accrueTo_, notional[ip * deltaN] * rec_pay.RecSign(), dct));
	}
	return retval;
}


