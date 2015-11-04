
// Bogaert's fastgl work, plus an adapter for our quadrature

#pragma once

#include "Quadrature.h"

// Functions for fastgl in double precision
namespace fastgl {
	// A struct for containing a Node-Weight pair
	struct QuadPair {
		double theta, weight;

		// A function for getting the node in x-space
		double x() const;

		// A constructor
		QuadPair(double t, double w) : theta(t), weight(w) {}
		QuadPair() {}
	};

	// Function for getting Gauss-Legendre nodes & weights
	// Theta values of the zeros are in [0,pi], and monotonically increasing. 
	// The index of the zero k should always be in [1,n].
	// Compute a node-weight pair:
	QuadPair GLPair(size_t n, int k);
}

namespace Quadrature
{
	void GaussLegendreWeights(double lo, double hi, Vector_<>* x, Vector_<>* w);
}

template<class T_ = double> class QuadGaussLegendre_ : public Quad1DFixed_<T_>
{
public:
	QuadGaussLegendre_(int n, double lo, double hi, const T_& initial = 0.0) : Quad1DFixed_<T_>(n, initial)
	{
		Quadrature::GaussLegendreWeights(lo, hi, &x_, &w_);
	}
};
