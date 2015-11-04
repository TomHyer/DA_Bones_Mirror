
#include "Platform.h"
#include "Quadrature.h"
#include "Strict.h"

Quad1DBase_::~Quad1DBase_()
{	}

void Quadrature::SimpsonWeights(int n, double lo, double hi, Vector_<>* x, Vector_<>* w)
{
   n |= 1; // need n to be odd
   const double dx = (hi - lo) / (n - 1);
   for (int ii = 0; ii < n; ++ii)
   {
      (*x)[ii] = lo + ii * dx;
      (*w)[ii] = (ii & 1 ? 4 : 2) * dx / 3.0;
   }
   // fix endpoints
   w->front() = w->back() = dx / 3.0;
}

//static const QuadSimpson_<Vector_<>> THING(11, 0.0, 1.0, Vector_<>(5, 0.0));