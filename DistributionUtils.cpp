
#include "Platform.h"
#include "DistributionUtils.h"
#include "Strict.h"

#include "OptionType.h"
#include "DistributionBlack.h"

double Distribution::BlackIV
	(const Distribution_& model, 
	 double strike, 
	 double guess, 
	 int n_steps)
{
	const double f = model.Forward();
	const OptionType_ type = strike > f
			? OptionType_::Value_::CALL
			: OptionType_::Value_::PUT;

	if (n_steps > 1)
	{
		const double fMid = strike > f
				? strike * pow(f / strike, 1.0 / n_steps)
				: strike + (f - strike) / n_steps;
		guess = BlackIV(model, fMid, guess, n_steps - 1);
	}
	return BlackIV
		(f, strike, type, model.OptionPrice(strike, type), guess);
}
