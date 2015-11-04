
#include "Platform.h"
#include "Trade.h"
#include "Strict.h"

Handle_<Trade_> TradeData_::Parse() const
{
	// protection here if multithreading
	if (parsed_.Empty())
		parsed_.reset(XParse());
	return parsed_;
}

Trade_::Trade_
	(const Vector_<String_>& value_names,
	 const Underlying_& underlying,
	 const Ccy_& value_ccy,
	 const CollateralType_& collateral)
:
valueNames_(value_names),
underlying_(underlying),
valueCcy_(value_ccy),
collateral_(collateral)
{	}

