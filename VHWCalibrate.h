
// calibrators creating VHW models

#pragma once

#include "VHW.h"

class Swaption_;

namespace VHW
{
	Model_* MatchSwaptionHoLee
		(const String_& name,
		 const Handle_<YieldCurve_>& yc, 
		 const Swaption_& swaption, 
		 double value,
		 const DateTime_& vol_start);
}

