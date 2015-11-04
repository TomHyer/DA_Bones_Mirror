
// base class to represent a coupon rate for one period in a leg

#pragma once

#include "DateTime.h"
#include "Currency.h"
#include "Vectors.h"

class PeriodLength_;
class Clearer_;

struct CouponRate_ : noncopyable
{
	virtual ~CouponRate_();
};

struct FixedRate_ : CouponRate_
{
	double rate_;
	FixedRate_(double rate) : rate_(rate) {}
};

/*IF--------------------------------------------------------------------------
enumeration TradedRate
   Quantities for which forecast curves are formed
switchable
alternative LIBOR_3M_CME
   3M Libor underlying CME swaps
alternative LIBOR_3M_LCH
   3M Libor underlying LCH swaps
alternative LIBOR_3M_FUT
   3M Libor underlying futures
alternative LIBOR_6M_CME
   6M Libor underlying CME swaps
alternative LIBOR_6M_LCH
   6M Libor underlying LCH swaps
method PeriodLength_ Period() const;
method Clearer_ Clearer() const;
-IF-------------------------------------------------------------------------*/

#include "MG_TradedRate_enum.h"

TradedRate_ FindRate(const PeriodLength_& period, const Clearer_& ch);

struct LiborRate_ : CouponRate_
{
	DateTime_ fixDate_;
	Ccy_ ccy_;
	TradedRate_ rate_;	// should be parseable as Date::Increment_
	LiborRate_(const DateTime_& fix_date, const Ccy_& ccy, const TradedRate_& rate);
};

// linear combination of underlying rates (e.g., margin or interpolated Libor)
struct SummedRate_ : CouponRate_
{
	Vector_<pair<double, Handle_<CouponRate_> > > rates_;
};

