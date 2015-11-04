
// band-diagonal matrices

#pragma once

#include "Matrix.h"

class SquareMatrixDecomposition_;

namespace Sparse
{
	class Square_;

	Square_* NewBandDiagonal
		(int size, int n_above, int n_below);
}

class LowerBandAccumulator_
{
	Matrix_<> vals_;
public:
	LowerBandAccumulator_(int size, int n_below);
	void Add(const Vector_<>& v, int offset);	// v.size() <= n_below + 1; it gives (at the indicated offset) the nonzero elements of a vector V.  We change our stored L to add V^T V to L^T L

	void SolveLeft(const Vector_<>& b, Vector_<>* x) const;	// such that Ax = b
	void SolveRight(const Vector_<>& b, Vector_<>* x) const;	// such that xA = b
};

