
// conjugate and bi-conjugate gradient solvers

#pragma once

class HasPreconditioner_
{
public:
	virtual void PreconditionerSolveLeft
		(const Vector_<>& b, Vector_<>* x) const = 0;
	virtual void PreconditionerSolveRight
		(const Vector_<>& b, Vector_<>* x) const = 0;
};

namespace Sparse
{
	class Square_;

	void CGSolve
		(const Sparse::Square_& A,
		 const Vector_<>& b,
		 double tol_rel,
       double tol_abs,
		 int max_iterations,
		 Vector_<>* x);
}

