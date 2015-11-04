
// decompositions, generally of square matrices, for numerical linear algebra

#pragma once

#include "Vectors.h"

class SquareMatrixDecomposition_ : noncopyable
{
	virtual void XMultiplyLeft_af
		(const Vector_<>& x,
		 Vector_<>* b) 
	const = 0;
	virtual void XMultiplyRight_af
		(const Vector_<>& x,
		 Vector_<>* b) 
	const = 0;
	virtual void XSolveLeft_af
		(const Vector_<>& b,
		 Vector_<>* x) 
	const = 0;
	virtual void XSolveRight_af
		(const Vector_<>& b,
		 Vector_<>* x) 
	const = 0;
public:  
	virtual ~SquareMatrixDecomposition_() {}
	virtual int Size() const = 0;    // of the matrix
	// these nonvirtual methods handle aliasing:
	void MultiplyLeft(const Vector_<>& x, Vector_<>* b) const;
	void MultiplyRight(const Vector_<>& x, Vector_<>* b) const;
	void SolveLeft(const Vector_<>& b, Vector_<>* x) const;
	void SolveRight(const Vector_<>& b, Vector_<>* x) const;
};

// special case of symmetric matrix:

class SymmetricMatrixDecomposition_	: public SquareMatrixDecomposition_
{
	virtual void XMultiply_af
		(const Vector_<>& x, 
		 Vector_<>* b) 
	const = 0;

	void XMultiplyLeft_af(const Vector_<>& x, Vector_<>* b) const override {	XMultiply_af(x, b);	}
	void XMultiplyRight_af(const Vector_<>& x, Vector_<>* b) const override { XMultiply_af(x, b); }

	virtual void XSolve_af
		(const Vector_<>& b, 
		 Vector_<>* x) 
	const = 0;
	void XSolveLeft_af(const Vector_<>& b, Vector_<>* x) const override { XSolve_af(b, x); }
	void XSolveRight_af(const Vector_<>& b, Vector_<>* x) const override { XSolve_af(b, x); }

public:
	virtual int Rank() const { return Size(); }
	virtual Vector_<>::const_iterator MakeCorrelated
		(Vector_<>::const_iterator iid_begin,
		 Vector_<>* correlated)
	const = 0;
	// These nonvirtual methods handle aliasing:
	void Multiply(const Vector_<>& x, Vector_<>* b) const;
	void Solve(const Vector_<>& b, Vector_<>* x) const;
};

class ExponentiatesMatrix_
{
public:
	virtual void ExpAT
		(double t, 
		 SquareMatrix_<>* dst)
	const = 0;
};

