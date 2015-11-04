
// piecewise-linear function of date

#pragma once

#include "Vectors.h"
#include "Date.h"

struct PiecewiseLinear_
{
	Vector_<Date_> knotDates_;
	Vector_<> fLeft_;
	Vector_<> fRight_;
	Vector_<> sofar_;	// precomputed integrals to knot dates

	// compute sofar_ (e.g. after a change)
	Vector_<> Sofar() const;
	void Update() { sofar_ = Sofar(); }
	PiecewiseLinear_(const Vector_<Date_>& knots, const Vector_<>& f_left, const Vector_<>& f_right) : knotDates_(knots), fLeft_(f_left), fRight_(f_right) { Update(); }

	double IntegralTo(const Date_& date) const;
   double ValueAt(const Date_& date, bool from_right = true) const;
};
