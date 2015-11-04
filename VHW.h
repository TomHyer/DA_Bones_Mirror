
// Vasicek-Hull-White model

#pragma once

#include "Model.h"
class DateTime_;

namespace VHW
{
	Model_* NewHoLee
		(const String_& name,
		 const Handle_<YieldCurve_>& yc,
		 const DateTime_& vol_start,
		 double vol);
}

