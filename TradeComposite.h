
// trades composed of (linear combinations of) other trades
	// e.g. an executed trade might be a multiple of some standard instrument
// also allows dividing value into multiple outputs
	// e.g. we might do a single trade with multiple counterparties

#pragma once

#include "Trade.h"

// mixin to identify/query composite trades

class IsCompositeTrade_
{
public:
	virtual Vector_<Handle_<Trade_>> SubTrades() const = 0;
	virtual Vector_<pair<String_, double> > FinalValues
		(const Vector_<pair<String_, double> >& component_vals)
	const = 0;
};

namespace Trade
{
   TradeData_* NewSum
      (const String_& name,
       const Vector_<Handle_<TradeData_>>& components);
}