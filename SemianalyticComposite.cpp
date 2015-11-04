
#include "Platform.h"
// this source file has no corresponding header
#include "Strict.h"

#include "Semianalytic.h"
#include "TradeComposite.h"

namespace
{
	struct PriceComposite_ : SemianalyticPricer_
	{
		bool Attempt
			(_ENV, const Trade_& trade,
			 const Model_& model,
			 const ValuationParameters_* params,
			 Vector_<pair<String_, double> >* vals)
		const override
		{
			auto composite = dynamic_cast<const IsCompositeTrade_*>(&trade);
			if (!composite)
				return false;
			for (const auto& s : composite->SubTrades())
				Append(vals, Semianalytic::Value(_env, *s, model, params));
			*vals = composite->FinalValues(*vals);
			return true;
		}
	};

	RUN_AT_LOAD(Semianalytic::Register(new PriceComposite_, 9))
}	// leave local

