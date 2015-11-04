
// Cholesky decomposition of symmetric matrices

#pragma once

class SymmetricMatrixDecomposition_;

SymmetricMatrixDecomposition_* CholeskyDecomposition
   (const SquareMatrix_<>& src);

// one-shot Cholesky solver, destroys the input matrix
void CholeskySolve
   (SquareMatrix_<>* a,  // will be decomposed in place
    Vector_<Vector_<>>* b, // each will be replaced with the Cholesky-solved version
	double regularization = DA::EPSILON);