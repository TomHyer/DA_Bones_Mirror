
#include "__Platform.h"
#include "Functionals.h"
#include "Trade.h"
#include "Model.h"
#include "Semianalytic.h"
#include "ValuationMethod.h"
#include "Globals.h"

namespace
{ 
/*IF------------------------------------------------------------------------
public Price
	Computes the value of a trade given a model
&inputs
trade is handle TradeData
	The trade to price
model is handle Model
	The model for valuation
&optional
params is settings ValuationParameters
	Settings controlling the valuation process
&outputs
values is number[]
	The values (maybe more than one) produced by this trade
value_names is string[]
	A name corresponding to each value
-IF-------------------------------------------------------------------------*/

	void Price
		(const Handle_<TradeData_>& trade,
		 const Handle_<Model_>& model,
		 const ValuationParameters_& params,
		 Vector_<>* values,
		 Vector_<String_>* value_names)
	{
		ENV_SEED_TYPE(Global::Dates_);

		Vector_<pair<String_, double>> namedVals;
		switch (params.method_.Switch())
		{
		case ValuationMethod_::Value_::CLOSED_FORM:
			namedVals = Semianalytic::Value(_env, *trade->Parse(), *model, &params);
			break;
		default:
			THROW("Numerical pricing does not exist");
		}
		*value_names = Apply(GetFirst(namedVals[0]), namedVals);
		*values = Apply(GetSecond(namedVals[0]), namedVals);
	}
}	// leave local

#include "MG_Price_public.inc"

