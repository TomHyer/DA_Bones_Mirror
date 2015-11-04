
#include "Platform.h"
#include "LegTrade.h"
#include "Strict.h"

#include "Flow.h"
#include "Archive.h"
#include "ReceivePay.h"
#include "LegParams.h"
#include "Trade.h"
#include "LegSchedule.h"
#include "LegBased.h"
#include "CashTrade.h"
#include "IndexIr.h"

namespace
{
	Underlying_ LegUnderlying
		(const Ccy_& ccy,
		 const Vector_<LegPeriod_>& leg)
	{
		Underlying_ retval;
		for (const auto& p : leg)
		{
			retval.Include(ccy, p.payDate_);
			if (auto pl = handle_cast<LiborRate_>(p.rate_))
			{
				Handle_<Index_> index(new Index::Libor_(ccy, pl->rate_));
				// POSTPONED -- for efficiency, avoid re-creating the index for each period
				retval.Include(IndexKey_(index), pl->fixDate_);
			}
		}
		return retval;
	}

	class LegTrade_ : public Trade_, public IsLegs_
	{
		Vector_<LegPeriod_> leg_;
		const Ccy_& Ccy() const { return underlying_.payCcys_.begin()->first; }	// there can be only one
	public:
		LegTrade_
			(const String_& name, 
			 const Ccy_& ccy, 
			 const Vector_<LegPeriod_>& leg, 
			 const CollateralType_& collateral)
		:
		Trade_(Vector::V1(name), LegUnderlying(ccy, leg), ccy, collateral),
		leg_(leg)
		{	}

		Payout_* MakePayout
			(const ValuationParameters_&,
			 ValueRequest_& value_request)
		const override
		{
			static const Handle_<LegBased::MakeRate_> DMR;
			std::unique_ptr<LegBased::Payout_> retval(new LegBased::Payout_(valueNames_[0]));
			LegBased::MakeCoupon_ makeCoupon(value_request, DMR, valueNames_[0], Ccy());
			for (const auto& p : leg_)
				*retval += makeCoupon(p);
			return retval.release();
		}

		Vector_<LegPeriod_> Periods(const Ccy_& pay_ccy) const override
		{
			return pay_ccy == Ccy() ? leg_ : Vector_<LegPeriod_>();
		}
	};
}

//----------------------------------------------------------------------------

// an actual trade:  fixed leg
namespace
{
/*IF--------------------------------------------------------------------------
storable FixedLegTrade
version 1
&members
name is ?string
ccy is enum Ccy
terms is handle LegScheduleParams
	Parameters from which leg is built
recPay is enum RecPay
	Sign of flows
notional is number[]
	Single or per-period notional
couponRate is number[]
	Single or per-period coupon
collateral is enum CollateralType
notionalExchange is ?enum NotionalExchange
	States whether/when notionals are exchanged
-IF-------------------------------------------------------------------------*/

#include "MG_FixedLegTrade_v1_Write.inc"
	Flow_ CashflowOfFixedPeriod(const LegPeriod_& period)
	{
		Flow_ retval;
		retval.payDate_ = period.payDate_;
		auto fixed = handle_cast<FixedRate_>(period.rate_);
		assert(fixed);
		retval.amount_ = period.accrual_->notional_ * fixed->rate_ * period.accrual_->dcf_;
		return retval;
	}

	struct FixedLegTrade_ : TradeData_
	{
		Ccy_ ccy_;
		Handle_<LegScheduleParams_> terms_;
		RecPay_ recPay_;
		Vector_<> notional_;
		Vector_<> couponRate_;
		CollateralType_ collateral_;
		NotionalExchange_ notionalExchange_;

		FixedLegTrade_
			(const String_& name,
			 const Ccy_& ccy,
			 const Handle_<LegScheduleParams_>& terms,
			 const RecPay_& rec_pay,
			 const Vector_<>& notional,
			 const Vector_<>& coupon_rate,
			 const CollateralType_& collateral,
			 const NotionalExchange_& notional_exchange = NotionalExchange_())
		: TradeData_(name), ccy_(ccy), terms_(terms), recPay_(rec_pay), notional_(notional), couponRate_(coupon_rate), collateral_(collateral), notionalExchange_(notional_exchange)                                                         
		{	}

		void Write(Archive::Store_& dst) const
		{
			FixedLegTrade_v1::XWrite(dst, name_, ccy_, terms_, recPay_, notional_, couponRate_, collateral_, notionalExchange_);
		}

		Trade_* XParse() const
		{
			auto periods = LegBuild::Fixed(ccy_, *terms_, recPay_, notional_, couponRate_, &notionalExchange_);
//			auto flows = Apply(CashflowOfFixedPeriod, periods);
//			return NewCashTradeImp(name_, ccy_, flows, collateral_);
			return new LegTrade_(name_, ccy_, periods, collateral_);
		}
	};

#include "MG_FixedLegTrade_v1_Read.inc"
}

TradeData_* NewFixedLeg
	(const String_& name,
	 const Ccy_& ccy,
	 const Handle_<LegScheduleParams_>& terms, 
	 const RecPay_& rec_pay,
	 const Vector_<>& notional,
	 const Vector_<>& coupon,
	 const CollateralType_& collateral)
{
	return new FixedLegTrade_(name, ccy, terms, rec_pay, notional, coupon, collateral);
}

// Libor float leg, very similar to fixed
namespace
{
/*IF--------------------------------------------------------------------------
storable LiborLegTrade
version 1
&members
name is ?string
ccy is enum Ccy
terms is handle LegScheduleParams
	Parameters from which leg is built
recPay is enum RecPay
	Sign of flows
notional is number[]
	Single or per-period notional
liborRate is enum TradedRate
	The Libor rate to pay
collateral is enum CollateralType
notionalExchange is ?enum NotionalExchange
	States whether/when notionals are exchanged
-IF-------------------------------------------------------------------------*/

#include "MG_LiborLegTrade_v1_Write.inc"

	LiborFlow_ CashflowOfLiborPeriod(const LegPeriod_& period)
	{
		LiborFlow_ retval;
		retval.payDate_ = period.payDate_;
		retval.leverage_ = period.accrual_->notional_ * period.accrual_->dcf_;
		auto libor = handle_cast<LiborRate_>(period.rate_);
		assert(libor);
		retval.rate_ = libor->rate_;
		return retval;
	}

	struct LiborLegTrade_ : TradeData_
	{
		Ccy_ ccy_;
		Handle_<LegScheduleParams_> terms_;
		RecPay_ recPay_;
		Vector_<> notional_;
		TradedRate_ liborRate_;
		CollateralType_ collateral_;
		NotionalExchange_ notionalExchange_;

		LiborLegTrade_
			(const String_& name,
			 const Ccy_& ccy,
			 const Handle_<LegScheduleParams_>& terms,
			 const RecPay_& rec_pay,
			 const Vector_<>& notional,
			 const TradedRate_& rate,
			 const CollateralType_& collateral,
			 const NotionalExchange_& notional_exchange = NotionalExchange_())
		: TradeData_(name), ccy_(ccy), terms_(terms), recPay_(rec_pay), notional_(notional), liborRate_(rate), collateral_(collateral), notionalExchange_(notional_exchange)                                                         
		{	}

		void Write(Archive::Store_& dst) const
		{
			LiborLegTrade_v1::XWrite(dst, name_, ccy_, terms_, recPay_, notional_, liborRate_, collateral_, notionalExchange_);
		}

		Trade_* XParse() const
		{
			auto periods = LegBuild::Libor(ccy_, *terms_, recPay_, notional_, Vector_<>(), nullptr, Vector_<int>(), &notionalExchange_);	// POSTPONED -- margin, interp libor
			return new LegTrade_(name_, ccy_, periods, collateral_);
		}
	};

#include "MG_LiborLegTrade_v1_Read.inc"
}

TradeData_* NewLiborLeg
	(const String_& name,
	 const Ccy_& ccy,
	 const Handle_<LegScheduleParams_>& terms, 
	 const RecPay_& rec_pay,
	 const Vector_<>& notional,
	 const TradedRate_& libor_rate,
	 const CollateralType_& collateral)
{
	return new LiborLegTrade_(name, ccy, terms, rec_pay, notional, libor_rate, collateral);
}

