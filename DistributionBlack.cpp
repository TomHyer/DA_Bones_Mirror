
#include "Platform.h"
#include "DistributionBlack.h"
#include "Strict.h"

#include "SpecialFunctions.h"
#include "OptionType.h"
#include "Exceptions.h"
#include "Rootfind.h"

double Distribution::BlackOpt
	(double fwd,
	 double vol,
	 double strike,
	 const OptionType_& type)
{
	if (IsZero(vol) || !IsPositive(fwd * strike))
		return type.Payout(fwd, strike);
	const double dMinus = log(fwd / strike) / vol - 0.5 * vol;
	const double dPlus = dMinus + vol;
	switch (type.Switch())
	{
	default:
		assert(!"Invalid OptionType");
	case OptionType_::Value_::CALL:
		return fwd * NCDF(dPlus) - strike * NCDF(dMinus);
	case OptionType_::Value_::PUT:
		return strike * NCDF(-dMinus) - fwd * NCDF(-dPlus);
	case OptionType_::Value_::STRADDLE:
		return fwd * (1.0 - 2.0 * NCDF(-dPlus)) + strike * (1.0 - 2.0 * NCDF(dMinus));
	}
	assert(!"Impossible option type");
	return 0.0;
}

double Distribution::BlackIV
	(double fwd,
	 double strike,
	 const OptionType_& type,
	 double price,
	 double guess)
{
	static const int MAX_ITERATIONS = 30;
	static const double TOL = 1.0e-10;	// see use below
	REQUIRE(price >= type.Payout(fwd, strike), "Value below intrinsic in BlackIV");
	// not very much error checking here, so stupid parameters generally result in a rootfinder failure
	Brent_ task(guess ? log(guess) : -1.5);
	Converged_ done(TOL * Max(1.0, fwd), TOL * Max(1.0, price));
	for (int i = 0; i < MAX_ITERATIONS; ++i)
	{
		const double vol = exp(task.NextX());
		if (done(task, BlackOpt(fwd, vol, strike, type) - price))
			return vol;
	}
	THROW("Exhausted iterations in BlackIV");
}

