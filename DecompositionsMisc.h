
// miscellaneous square-matrix decompositions

#pragma once

class SquareMatrixDecomposition_;
class SymmetricMatrixDecomposition_;

SymmetricMatrixDecomposition_* DiagonalAsDecomposition
	(const Vector_<>& diag);
SquareMatrixDecomposition_* LowerTriangularAsDecomposition
	(const SquareMatrix_<>& src);
