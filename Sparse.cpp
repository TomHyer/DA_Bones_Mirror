
#include "Platform.h"
#include "Sparse.h"
#include "Strict.h"

#include "SquareMatrix.h"
#include "Numerics.h"

// brute-force implementation
void Sparse::SymmetricDecomposition_::QForm
	(const Matrix_<>& j,
	 SquareMatrix_<>* form)
const
{
	form->Resize(j.Rows());
   Vector_<> wij; // one row of W^{-1} J^T
	Vector_<> row(j.Cols());
	for (int ii = 0; ii < j.Rows(); ++ii)
	{
		Copy(j.Row(ii), &row);
		Solve(row, &wij);
		for (int jj = 0; jj <= ii; ++jj)
			(*form)(ii, jj) = (*form)(jj, ii) = InnerProduct(wij, j.Row(jj));
	}
}

Sparse::SymmetricDecomposition_* Sparse::Square_::DecomposeSymmetric() const
{
   std::unique_ptr<SquareMatrixDecomposition_> d(Decompose());
   if (auto retval = dynamic_cast<Sparse::SymmetricDecomposition_*>(d.get()))
   {
      d.release();
      return retval;
   }
   // cast failed
   assert(!IsSymmetric()); // symmetric matrix should return a type that implements QForm
   return nullptr;
}

