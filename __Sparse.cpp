
// Public access to sparse matrix interface

#include "__Platform.h"
#include "Sparse.h"
#include "SparseUtils.h"
#include "Banded.h"
#include "BlockDiagonal.h"
#include "SLAP.h"

/*IF--------------------------------------------------------------------------
enumeration SparseType
switchable
alternative BANDED
alternative SLAP
-IF-------------------------------------------------------------------------*/
#include "MG_SparseType_enum.h"
#include "MG_SparseType_enum.inc"

namespace
{
/*IF--------------------------------------------------------------------------
public Sparse_Solve1
   Solves a sparse matrix equation
&inputs
m is number[][]
   &$.Rows() == $.Cols()\$ must be square
   The matrix, hopefully sparse
v is number[]
   &$.size() == m.Rows()\$ must have the same size as m
   The vector to solve
type is enum SparseType
   The type of sparse matrix to set up
&outputs
b is number[]
   Such that m b = v
-IF-------------------------------------------------------------------------*/

   void Sparse_Solve1
      (const Matrix_<>& m,
      const Vector_<>& v,
      const SparseType_& type,
      Vector_<>* b)
   {
      const int n = m.Rows();
      // count width of band
      int nAbove = 0, nBelow = 0;
      for (int ii = 0; ii < n; ++ii)
      {
         for (int jj = 0; jj < ii; ++jj)
         {
            if (!IsZero(m(ii, jj)))
               nBelow = Max(nBelow, ii - jj);
            if (!IsZero(m(jj, ii)))
               nAbove = Max(nAbove, ii - jj);
         }
      }
      std::unique_ptr<Sparse::Square_> w;
      switch (type.Switch())
      {
      case SparseType_::Value_::BANDED:
         w.reset(Sparse::NewBandDiagonal(n, nAbove, nBelow));
         break;
//      case SparseType_::Value_::SLAP:
//         w.reset(Sparse::NewSLAP(n));
//         break;
      default:
         THROW("Invalid sparse matrix type");
      }

      // populate the matrix
      for (int ii = 0; ii < n; ++ii)
      for (int jj = 0; jj < n; ++jj)
      if (!IsZero(m(ii, jj)))
         w->Set(ii, jj, m(ii, jj));
      // decompose and solve
      scoped_ptr<Sparse::SymmetricDecomposition_> d(w->DecomposeSymmetric());
      d->Solve(v, b);
   }
}  // leave local

#include "MG_Sparse_Solve1_public.inc"
