
// numerical integration

#pragma once

#include "Algorithms.h"
#include "Functionals.h"
#include "Vectors.h"

// helpers for integration of vector-valued functions
namespace Quadrature
{
	template<class T_> inline void Increment
		(T_* dst, const T_& inc, double w)
	{
		T_ z(inc); z *= w; *dst += z;
	}
	template<> inline void Increment   // specialization
		(Vector_<>* dst, const Vector_<>& inc, double w)
	{
		Transform(dst, inc, LinearIncrement(w));
	}

   // particular distributions of interest
   void NCDFGaussHermiteWeights(Vector_<>* x, Vector_<>* w);
   void SimpsonWeights(int n, double lo, double hi, Vector_<>* x, Vector_<>* w);
}

// base class for out-of-line destructor
class Quad1DBase_
{
public:
	virtual ~Quad1DBase_();
};

template<class T_> class Quad1D_ : public Quad1DBase_
{
public:
	virtual double GetX() = 0;
	virtual void PutY(const T_& y) = 0;
	virtual bool IsComplete() const = 0;
	virtual T_ Result() const = 0;
	virtual void Restart() = 0;
};

template<class T_> class Quad1DFixed_ : public Quad1D_<T_>
{
	size_t i_;   // because STL sizes are size_t
	T_ sum_, initial_;
protected:
	Vector_<> x_, w_;
	Quad1DFixed_(int size, const T_& initial)
		: i_(0), sum_(initial), initial_(initial), x_(size), w_(size)
	{   }
public:
	double GetX() { assert(!IsComplete()); return x_[i_]; }
	void PutY(const T_& y)
	{
		assert(!IsComplete());
		Quadrature::Increment(&sum_, y, w_[i_++]);
	}
	bool IsComplete() const { return i_ == x_.size(); }
	T_ Result() const { assert(IsComplete()); return sum_; }
	void Restart() { i_ = 0; sum_ = initial_; }
	// allow query
	const Vector_<>& Abcissa() const { return x_; }
	const Vector_<>& Weight() const { return w_; }
};


template<class T_ = double> class NormalExpectation_ : public Quad1DFixed_<T_>
{
public:
	NormalExpectation_(int n, const T_& initial = 0.0)
		: Quad1DFixed_<T_>(n, initial)
	{
		Quadrature::NCDFGaussHermiteWeights(&x_, &w_);
	}
};

template<class T_ = double> class QuadSimpson_ : public Quad1DFixed_<T_>
{
public:
   QuadSimpson_(int n, double lo, double hi, const T_& initial = 0.0)
      : Quad1DFixed_<T_>(n | 1, initial)  // N must be odd
   {
      Quadrature::SimpsonWeights(n, lo, hi, &x_, &w_);
   }
};

