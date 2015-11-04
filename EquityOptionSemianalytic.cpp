
#include "Platform.h"
#include "EquityOptionSemianalytic.h"
#include "Strict.h"

#include "EquityOption.h"
#include "Model.h"
#include "Trade.h"
#include "Semianalytic.h"

struct PriceEquityOption_ : SemianalyticPricer_
{
	bool Attempt
		(_ENV, const Trade_& trade,
		 const Model_& model,
		 const ValuationParameters_*,
		 Vector_<pair<String_, double>>* vals)
	const
	{
		auto myOpt = dynamic_cast<const IsEquityOption_*>(&trade);
		auto myModel = dynamic_cast<const HasAnalyticEquity_*>(&model);
		if (!myModel || !myOpt)
			return false;
		vals->emplace_back(trade.valueNames_[0], myModel->Price(*myOpt));
		return true;
	}
};
RUN_AT_LOAD(Semianalytic::Register(new PriceEquityOption_, 5))
