
#include "Platform.h"
#include "SpecialFunctions.h"
#include <boost\math\distributions\normal.hpp>
#include "Strict.h"

#include "Vectors.h"
#include "InterpCubic.h"
#include "Strings.h"

namespace
{
	static const double MIN_SPLINE_X = -3.734582185;
	static const double MIN_SPLINE_F = 9.47235E-05;
	Interp1_* MakeNcdfSpline()
	{
		static const Vector_<> x = { MIN_SPLINE_X, -3.347382781, -3.030883722, -2.75090681, -2.492289824, -2.243141537,	-1.992179668, -1.494029881, 
				-1.290815576, -1.120050999, -0.954303629, -0.792072249,	-0.629093487, -0.460389924, -0.276889742, 0.0 };
		static const Vector_<> f = { MIN_SPLINE_F, 0.000408582, 0.001219907, 0.002972237, 0.00634685, 0.012444548, 0.023176395, 0.067583453,
				0.098383227, 0.131345731, 0.16996458, 0.214158839, 0.264643073, 0.322617682, 0.39093184, 0.5 };
		const Interp::Boundary_ lhs(1, 0.000373538);
		const Interp::Boundary_ rhs(1, 0.39898679); // at x=0
		return Interp::NewCubic(String_(), x, f, lhs, rhs);
	}

	double NcdfBySpline(double z)
	{
		static const scoped_ptr<Interp1_> SPLINE(MakeNcdfSpline());
		if (z > 0.0)
			return 1.0 - NcdfBySpline(-z);
		return z < MIN_SPLINE_X
				? MIN_SPLINE_F * exp(-1.1180061 * (Square(z) - Square(MIN_SPLINE_X)))
				: (*SPLINE)(z);
	}
}

double NCDF(double x, bool precise)
{
	static const boost::math::normal_distribution<double> PDF;
	return precise
			? boost::math::cdf(PDF, x)
			: NcdfBySpline(x);
}

double InverseNCDF(double x, bool precise, bool polish)
{
	static const double INV_NORM = sqrt(2.0 * DA::PI);
	static const boost::math::normal_distribution<double> PDF;
	assert(x >= 0.0 && x <= 1.0);
	double retval = boost::math::quantile(PDF, DA::EPSILON + x * (1.0 - 2.0 * DA::EPSILON));
	if (polish)
	{
		const double err = NCDF(retval, precise) - x;
		retval -= err * INV_NORM * exp(Min(8.0, 0.5 * Square(retval)));	// cap exp(x^2) factor in polishing at 4 sigma
	}
	return retval;
}

