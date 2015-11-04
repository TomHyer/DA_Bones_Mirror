
// square matrix -- relies on Matrix_ for most implementation

#pragma once

#include "Matrix.h"

template<class E_> class SquareMatrix_
{
	Matrix_<E_> val_;
public:
   SquareMatrix_() {}
	SquareMatrix_(int size) : val_(size, size) {}
	void Resize(int size) { val_.Resize(size, size); }
	
	operator const Matrix_<E_>&() const { return val_; }
	// conversion to Matrix_ lets us use non-member functions taking a Matrix_ input, but we need to re-publish members
	double& operator()(int i, int j) { return val_(i, j); }
	const double& operator()(int i, int j) const
	{
		return val_(i, j);
	}
	int Rows() const { return val_.Rows(); }
	Matrix_<>::Row_ Row(int ii) { return val_.Row(ii); }
	Matrix_<>::ConstRow_ Row(int ii) const { return val_.Row(ii); }
	int Cols() const { return val_.Cols(); }
	Matrix_<>::Col_ Col(int ii) { return val_.Col(ii); }
	Matrix_<>::ConstCol_ Col(int ii) const { return val_.Col(ii); }
};

namespace SquareMatrix
{
	template<class E_> SquareMatrix_<E_> M1x1(const E_& val)
	{
		SquareMatrix_<E_> retval(1);
		retval(0, 0) = val;
		return retval;
	}
}

