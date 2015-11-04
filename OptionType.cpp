
#include "Platform.h"
#include "OptionType.h"
#include "Strict.h"
#include "Exceptions.h"

#include "MG_OptionType_enum.inc"

double OptionType_::Payout(double S, double K) const
{
	switch (Switch())
	{
	case Value_::CALL:
		return Max(0.0, S - K);
	case Value_::PUT:
		return Max(0.0, K - S);
	case Value_::STRADDLE:
		return fabs(S - K);
	}
	assert(!"Invalid OptionType");
	return 0.0;
}

