
#include "__Platform.h"
#include "Eispack.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Decompose_Eigen
   Decompose a symmetric matrix to eigenvalues and eigenvectors
&inputs
A is number[][]
   &$.Rows() == $.Cols()\$ must be square
   A symmetric matrix
&outputs
d is number[]
   The eigenvalues of A
z is number[][]
   A matrix whose columns are eigenvectors of A, corresponding to the eigenvalues in d
-IF-------------------------------------------------------------------------*/

   void Decompose_Eigen
      (const Matrix_<>& A,
       Vector_<>* d,
       Matrix_<>* z)
   {
      Eispack::RS(A, d, z);
   }
}  // leave local

#include "MG_Decompose_Eigen_public.inc"

