
// routines ported from the EISPACK Fortran source

#pragma once

class SymmetricMatrixDecomposition_;

namespace Eispack
{
	void RS
		(const Matrix_<>& a, // the matrix to decompose -- must be symmetric
		Vector_<>* w,             // will be populated with eigenvalues
		Matrix_<>* z);             // will be populated with eigenvectors

	void RG
		(const Matrix_<>& a, // the matrix to decompose -- must be square
		 Vector_<>* wr,             // will be populated with real parts of eigenvalues
		 Vector_<>* wi,             // will be populated with imaginary parts of eigenvalues
		 Matrix_<>* z);             // will be populated with eigenvectors -- one column for real, or two columns (real/imaginary) per conjugate pair of eigenvalues
}

SymmetricMatrixDecomposition_* NewEigenSystem
	(const Matrix_<>& a,
	 double discard_threshold = 0.0,	// discard eigenmodes with eigenvalues below this
	 double error_threshold = -DA::INFINITY);	// error on eigenvalues below this