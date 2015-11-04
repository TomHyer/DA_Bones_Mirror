
// base class for semianalytic pricers using knowledge of model internals to price trades

#pragma once

#include "Environment.h"

class String_;
class Trade_;
class Model_;
struct ValuationParameters_;

// derived classes may not contain member data!
struct SemianalyticPricer_    
{
    virtual bool Attempt
       (_ENV, const Trade_& trade,
        const Model_& model,
        const ValuationParameters_* params,	// may be null
        Vector_<pair<String_, double> >* vals)
    const = 0;
};

namespace Semianalytic
{
	void Register(SemianalyticPricer_* orphan_pricer, int priority);

	Vector_<pair<String_, double> > Value
		(_ENV, const Trade_& trade,
		 const Model_& model,
		 const ValuationParameters_* params = nullptr);
}

