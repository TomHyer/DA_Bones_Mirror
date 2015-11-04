
// baskets of correlated assets, and options thereon

#pragma once

namespace Basket
{
	void FitShiftedLognormal
		(double m1,
		 double m2,
		 double m3,
		 double* ln_part,
		 double* shift,
		 double* vol,
		 double* root = nullptr);
}

