
// piecewise-constant function of datetime

#pragma once

#include "Vectors.h"
#include "DateTime.h"
#include "Storable.h"

/*IF--------------------------------------------------------------------------
storable PiecewiseConstant
	Piecewise constant function of DateTime
version 1
&members
knotDates is datetime[]
fRight is number[]
	Function value effective on-or-after the corresponding knot date
name is ?string
-IF-------------------------------------------------------------------------*/

struct PiecewiseConstant_ : Storable_
{
	Vector_<DateTime_> knotDates_;
	Vector_<> fRight_;	// nothing to the left of the first knot
	Vector_<> sofar_;	// precomputed integrals to knot dates

	// compute sofar_ (e.g. after a change)
	Vector_<> Sofar() const;
	void Update() { sofar_ = Sofar(); }
	PiecewiseConstant_(const Vector_<DateTime_>& knots, const Vector_<>& f_right, const String_& name = String_()) : Storable_("PiecewiseConstant", name), knotDates_(knots), fRight_(f_right) { Update(); }

	double IntegralTo(const DateTime_& dt) const;
	void Write(Archive::Store_& dst) const override;
};

namespace PWC
{
	// at the knot point, returns the limit-from-above (f_right)
	double F
		(const PiecewiseConstant_& func,
		 const DateTime_& t,
		 bool* is_knot = nullptr);
	inline double Integral
		(const PiecewiseConstant_& func,
		 const DateTime_& from,
		 const DateTime_& to)
	{
		return func.IntegralTo(to) - func.IntegralTo(from);
	}

	inline PiecewiseConstant_* NewConstant
		(double val,
		 const DateTime_& from = DateTime::Minimum())
	{
		return new PiecewiseConstant_(Vector::V1(from), Vector::V1(val));
	}
}

