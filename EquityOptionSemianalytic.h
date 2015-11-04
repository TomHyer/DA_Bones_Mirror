
// framework for semianalytic pricing of trades which turn out to be equity options
	// individual models participate by deriving from HasAnalyticEquity_ and implementing its Price() method

#pragma once

#include "EquityOption.h"

class HasAnalyticEquity_	// mixin class (no destructor)
{
public:
	virtual double Price(const EquityOption::Data_& option) const = 0;
};