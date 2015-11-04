
// Derivatives trade

#pragma once

#include "Underlying.h"
#include "ValueRequest.h"
#include "Storable.h"
#include "Currency.h"
#include "CollateralType.h"

class Payout_;
struct ValuationParameters_;

// pricing interface
class Trade_ : noncopyable
{
public:
	const Vector_<String_> valueNames_;
	const Underlying_ underlying_;
	const Ccy_ valueCcy_;
	const CollateralType_ collateral_;

	Trade_
		(const Vector_<String_>& value_names,
		 const Underlying_& underlying,
		 const Ccy_& value_ccy,
		 const CollateralType_& collateral);

	virtual Payout_* MakePayout
		(const ValuationParameters_& parameters,
		 ValueRequest_& value_request)
	const = 0;
};

// persistence interface
struct Bookkeeping_
{	};

class TradeData_ : public Storable_
{
	mutable Handle_<Trade_> parsed_;
	virtual Trade_* XParse() const = 0;
public:
	TradeData_(const String_& name) : Storable_("Trade", name) {}
	Handle_<Trade_> Parse() const;
	void Clear() const;   // un-Parse
	Bookkeeping_ Bookkeeping() const;
};
