
#include "Platform.h"
#include "Cap.h"
#include "Strict.h"

#include "Exceptions.h"
#include "OptionType.h"
#include "Archive.h"
#include "LegParams.h"
#include "Trade.h"

#include "MG_CapFloor_enum.inc"

OptionType_ CapFloor_::OptionOnRatesType() const
{
	switch (val_)
	{
	default:
		assert(!"Impossible cap/floor type");
	case Value_::CAP:
		return OptionType_::Value_::CALL;
	case Value_::FLOOR:
		return OptionType_::Value_::PUT;
	case Value_::STRADDLE:
		return OptionType_::Value_::STRADDLE;
	}
}

namespace
{
	// this implements the cap (option on rate), not the capped rate
	TradeAmount_* NewCapAmount
		(const Handle_<TradeAmount_>& base,
		 double strike,
		 const CapFloor_& type,
		 const BuySell_& buy_sell)
	{
		struct Mine_ : TradeAmount_
		{
			Handle_<TradeAmount_> base_;
			double strike_;
			OptionType_ type_;	// of option on rates
			int sign_;

			Mine_(const Handle_<TradeAmount_>& base, double strike, const OptionType_& type, int sign) : base_(base), strike_(strike), type_(type), sign_(sign) {}
			double operator()(const UpdateToken_& values) const override
			{
				return sign_ * type_.Payout((*base_)(values), strike_);
			}
		};
		return new Mine_(base, strike, type.OptionOnRatesType(), buy_sell.Sign());
	}
}	// leave local

pair<DateTime_, Handle_<TradeAmount_>> MakeRateCapped_::operator()
	(ValueRequest_& model, 
	 const CouponRate_& rate)
const
{
	if (auto cap = dynamic_cast<const CapRate_*>(&rate))
	{
		auto temp = (+base_)(model, *cap->underlying_);
		Handle_<TradeAmount_> capAmt
				(NewCapAmount
						(temp.second, cap->strike_, cap->type_, cap->sign_));
		return make_pair(temp.first, capAmt);
	}
	return (+base_)(model, rate);
}

namespace
{
/*IF--------------------------------------------------------------------------
storable LiborCapTrade
	Cap/floor on Libor
version 1
&members
name is ?string
ccy is enum Ccy
terms is handle LegScheduleParams
	From which leg is generated
fixingHols is string
	If omitted, currency defaults are used
fixingDelay is ?integer[]
	A single constant, or a delay for each fixing
buySell is enum BuySell
capFloor is enum CapFloor
notional is number[]
	A single constant, or a notional for each pay period
strike is number[]
	A single constant, or a strike for each period
-IF-------------------------------------------------------------------------*/

#include "MG_LiborCapTrade_v1_Write.inc"
	struct LiborCapTrade_ : TradeData_
	{
		Ccy_ ccy_;
		Handle_<LegScheduleParams_> terms_;
		String_ fixingHols_;
		Vector_<int> fixingDelay_;
		BuySell_ buySell_;
		CapFloor_ capFloor_;
		Vector_<double> notional_;
		Vector_<double> strike_;

		LiborCapTrade_
			(const String_& name,
			const Ccy_& ccy,
			const Handle_<LegScheduleParams_>& terms,
			const String_& fixing_hols,
			const Vector_<int>& fixing_delay,
			const BuySell_& buy_sell,
			const CapFloor_& cap_floor,
			const Vector_<double>& notional,
			const Vector_<double> strike)
			: TradeData_(name), ccy_(ccy), terms_(terms), fixingHols_(fixing_hols), fixingDelay_(fixing_delay), buySell_(buy_sell), capFloor_(cap_floor), notional_(notional), strike_(strike)
		{	}

		void Write(Archive::Store_& dst) const
		{
         LiborCapTrade_v1::XWrite(dst, name_, ccy_, terms_, fixingHols_, fixingDelay_, buySell_, capFloor_, notional_, strike_);
		}

		Trade_* XParse() const
		{
			return nullptr;	// postponed -- implement the cap trade
		}
	};

#include "MG_LiborCapTrade_v1_Read.inc"
}	// leave local
