
// smoothing splines

#pragma once

// raw function to compute z-vals used in smoothing spline
	// assumes x are sorted and distinct
Vector_<> SmoothedVals
	(const Vector_<>& x,
	 const Vector_<>& y,
	 const Vector_<>& weight,	// if empty, all weights are 1.0
	 double lambda);
