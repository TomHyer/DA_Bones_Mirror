
// sparse matrix routines

#pragma once

#include "Decompositions.h"

namespace Sparse
{
	class SymmetricDecomposition_ : public SymmetricMatrixDecomposition_
	{
	public:
		// form J^T A^{-1} J for given J
		virtual void QForm
			(const Matrix_<>& J,
			 SquareMatrix_<>* dst) 
		const;
	};

	class Square_ : noncopyable
	{
	public:
		virtual int Size() const = 0;

		virtual void MultiplyLeft
			(const Vector_<>& x, Vector_<>* b) const = 0;
		virtual void MultiplyRight
			(const Vector_<>& x, Vector_<>* b) const = 0;

		virtual bool IsSymmetric() const = 0;
		virtual SquareMatrixDecomposition_* Decompose() const = 0;	// must return a SymmetricDecomposition_ if possible
      Sparse::SymmetricDecomposition_* DecomposeSymmetric() const;  // casts and checks the result of Decompose; returns nullptr if not symmetric

		// element access
		virtual const double& operator()(int i_row, int i_col) const = 0;
		virtual void Set(int i_row, int i_col, double val) = 0;
		virtual void Add(int i_row, int i_col, double val)
		{
			Set(i_row, i_col, val + operator()(i_row, i_col));
		}
	};
}