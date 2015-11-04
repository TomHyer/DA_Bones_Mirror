
#include "Platform.h"
#include "Cholesky.h"
#include "Strict.h"

#include "Functionals.h"
#include "SquareMatrix.h"

void CholeskySolve
   (SquareMatrix_<>* a,	// will be destroyed (upper half is source; lower half and diagonal will be replaced with Cholesky decomposition)
    Vector_<Vector_<>>* b,	// to be solved
	double regularization)
{
   const int n = a->Rows();
   assert(a->Cols() == n);
   double meanDiag = 0.0;
   for (int ii = 0; ii < n; ++ii)
   {
      auto rowI = a->Row(ii);
      auto beginI = rowI.begin();
      auto paij = beginI;
      for (int jj = 0; jj < ii; ++jj, ++paij)
      {
         const double lockedIn = std::inner_product(beginI, paij, a->Row(jj).begin(), 0.0);
         const double needMore = (*a)(jj, ii) - lockedIn;	// read from upper half
		 *paij = needMore == 0.0 
				? 0.0
				: needMore * (*a)(jj, jj) / (Square((*a)(jj, jj)) + Square(regularization * meanDiag));
      }
      const double lockedIn = std::inner_product(a->Row(ii).begin(), a->Row(ii).begin() + ii, a->Row(ii).begin(), 0.0);
      const double needMore = (*a)(ii, ii) - lockedIn;
      (*a)(ii, ii) = sqrt(Max(0.0, needMore));
	  meanDiag += ((*a)(ii, ii) - meanDiag) / (1.0 + ii);
   }
   // done decomposition; solve by backsubstitution
   // first precompute regularized inverses
   const double reg = Square(regularization * meanDiag);
   assert(reg > 0.0);	// will also fail if reg is NaN
   for (int ii = 0; ii < n; ++ii)
	   (*a)(ii, ii) /= reg + Square((*a)(ii, ii));
   for (int ib = 0; ib < b->size(); ++ib)
   {
      Vector_<>& bb = (*b)[ib];
      for (int ii = 0; ii < n; ++ii)	// L-solve with A
      {
         bb[ii] -= std::inner_product(bb.begin(), bb.begin() + ii, a->Row(ii).begin(), 0.0);
         bb[ii] *= (*a)(ii, ii);
      }
      for (int ii = n - 1; ii >= 0; --ii)	// L-transpose-solve
      {
        bb[ii] *= (*a)(ii, ii);
        std::transform(bb.begin(), bb.begin() + ii, a->Row(ii).begin(), bb.begin(), LinearIncrement(-bb[ii]));
      }
   }
}

