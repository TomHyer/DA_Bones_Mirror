
// some trades are equity options -- provide a query function for pricing/reporting

#pragma once

#include "DateTime.h"
#include "OptionType.h"

class Trade_;

namespace EquityOption
{
	struct Data_
	{
		DateTime_ expiry_;
		Date_ delivery_;
		double strike_;
		OptionType_ type_;
	};
}

// mixin class for equity options to identify themselves
   // inheritance from Data_ is truly suboptimal here
      // it dictates that the trade must store all the Data_ elements, in the prescribed way, upon its construction
   // we use it only because for the book, we want to simultaneously display the different ways trades and models might interact
class IsEquityOption_ : public EquityOption::Data_
{
};

