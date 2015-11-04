
// helper functions for creating and customizing underdetermined search

#pragma once

#include "Banded.h"
#include "DateTime.h"

namespace Underdetermined
{
	// add couplings to a sparse matrix
	void SelfCouplePWC
		(Sparse::Square_* weights,
		 const Vector_<DateTime_>& knots,
		 double tau_smoothing,
		 int offset = 0);

	Sparse::Tridiagonal_* WeightsPWC
		(const Vector_<DateTime_>& knots, double tau_s)
	{
		std::unique_ptr<Sparse::Tridiagonal_> retval(new Sparse::Tridiagonal_(knots.size()));
		SelfCouplePWC(retval.get(), knots, tau_s, 0);
		return retval.release();
	}
}