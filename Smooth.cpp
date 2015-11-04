
#include "Platform.h"
#include "Smooth.h"
#include <functional>
#include "Strict.h"

#include "Vectors.h"
#include "Algorithms.h"

// smoothing spline z-vals
Vector_<> SmoothedVals
	(const Vector_<>& x,
	 const Vector_<>& y,
	 const Vector_<>& weight,	// if empty, all weights are 1.0
	 double lambda)
{
	static const double DX_MIN = 1.0e-9;

	assert(IsMonotonic(x, std::less_equal<double>()));
	const int n = x.size();
	assert(y.size() == n && n > 1);
	// populate the couplings in advance
	Vector_<> coupling(n);
	for (int ii = 1; ii < n; ++ii)
		coupling[ii - 1] = lambda / Max(x[ii] - x[ii - 1], DX_MIN);
	coupling.back() = 0.0;	// simplifies loop

	Vector_<> beta(n), gamma(n);
	// initialize forward recursion
	beta[0] = (weight.empty() ? 1.0 : weight[0]) + coupling[0];
	gamma[0] = (weight.empty() ? 1.0 : weight[0]) * y[0];
	for (int ii = 1; ii < n; ++ii)
	{
		const double w = weight.empty() ? 1.0 : weight[ii];
		gamma[ii] = w * y[ii] + coupling[ii - 1] * gamma[ii - 1] / beta[ii - 1];
		beta[ii] = w + coupling[ii - 1] + coupling[ii] - Square(coupling[ii - 1]) / beta[ii - 1];	// at end of loop, coupling[ii] is zero because it doesn't exist
	}
	// backsubstitution for final result
	Vector_<> z(n);
	z.back() = gamma.back() / beta.back();
	for (int jj = n - 2; jj >= 0; --jj)
		z[jj] = (gamma[jj] + coupling[jj] * z[jj + 1]) / beta[jj];
	return z;
}

