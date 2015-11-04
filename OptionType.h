
#pragma once

#include "Vectors.h"
#include "Strings.h"

/*IF--------------------------------------------------------------------------
enumeration OptionType
	Call/put flag
switchable
alternative CALL C
alternative PUT P
alternative STRADDLE V C+P
method double Payout(double spot, double strike) const;
method OptionType_ Opposite() const;
-IF-------------------------------------------------------------------------*/

#include "MG_OptionType_enum.h"