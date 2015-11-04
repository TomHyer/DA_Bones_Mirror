
// caps on floating rates

#pragma once

#include "CouponRate.h"
#include "BuySell.h"
#include "LegBased.h"

class OptionType_;

/*IF--------------------------------------------------------------------------
enumeration CapFloor
	Distinguishes cap/floor types
alternative CAP C
	Call option on rates, or upper limit on floating rate
alternative FLOOR F
	Put option on rates, or lower limit on floating rate
alternative STRADDLE
method OptionType_ OptionOnRatesType() const;
-IF-------------------------------------------------------------------------*/
#include "MG_CapFloor_enum.h"

struct CapRate_ : CouponRate_
{
	Handle_<CouponRate_> underlying_;
	double strike_;
	CapFloor_ type_;
	BuySell_ sign_;
};

struct MakeRateCapped_ : LegBased::MakeRate_
{
	Handle_<MakeRate_> base_;
	MakeRateCapped_(const Handle_<MakeRate_>& base = Handle_<MakeRate_>()) : base_(base) {}

	pair<DateTime_, Handle_<TradeAmount_> > operator()
		(ValueRequest_& model, const CouponRate_& underlying_rate) 
   const override;
};
