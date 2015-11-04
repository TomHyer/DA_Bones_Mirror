
#include "Platform.h"
#include "BCG.h"
#include "Strict.h"

#include "Sparse.h"
#include "Algorithms.h"
#include "Functionals.h"
#include "Numerics.h"
#include "Exceptions.h"

namespace
{
	struct XPrecondition_
	{
		const HasPreconditioner_* a_;   // we do not own this
		XPrecondition_(const Sparse::Square_& a)
			: a_(dynamic_cast<const HasPreconditioner_*>(&a)) {}
		void Left(const Vector_<>& b, Vector_<>* x) const
		{
			if (a_)
				a_->PreconditionerSolveLeft(b, x);
			else if (x != &b)
				Copy(b, x);
		}
		void Right(const Vector_<>& b, Vector_<>* x) const
		{
			if (a_)
				a_->PreconditionerSolveRight(b, x);
			else if (x != &b)
				Copy(b, x);
		}
	};

	// called only from BCG; exists only to support preconditioner solve right
	struct XSparseTransposed_ : public Sparse::Square_, public HasPreconditioner_
	{
		const Sparse::Square_& a_;
		XPrecondition_ p_;
		XSparseTransposed_(const Sparse::Square_& a)
			: a_(a), p_(a) {}

		int Size() const { return a_.Size(); }
		void XMultiplyLeft_af(const Vector_<>& x,
			Vector_<>* b) const {
			a_.MultiplyRight(x, b);
		}
		void XSolveLeft_af(const Vector_<>& b,
			Vector_<>* x) const {
			assert(!"Unreachable:  left-solve after transpose");
		}
		void PreconditionerSolveLeft
			(const Vector_<>& x, Vector_<>* b) const
		{
			p_.Right(x, b);
		}
		// ... Right functions are all UNREACHABLE
	};
}	// leave local

void Sparse::CGSolve
	(const Sparse::Square_& A,
	 const Vector_<>& b,
	 double tol_rel,
    double tol_abs,
	 int max_iterations,
	 Vector_<>* x)
{
	const int n = A.Size();
	assert(b.size() == n && x->size() == n);
	assert((IsPositive(tol_rel) || IsPositive(tol_abs)) && max_iterations > 0);

	double tNorm = tol_rel * sqrt(InnerProduct(b, b)) + tol_abs;
	XPrecondition_ precondition(A);
	Vector_<> r(n), z(n), p(n);
	A.MultiplyLeft(*x, &r);
	Transform(b, r, std::minus<double>(), &r);  // r = b - Ax
	double betaPrev;
	for (int ii = 0; ii < max_iterations; ++ii)
	{
		precondition.Left(r, &z);
		const double beta = InnerProduct(z, r);
      p *= ii > 0 ? beta / betaPrev : 0.0;
      p += z;
		betaPrev = beta;
		A.MultiplyLeft(p, &z);
		const double alphaK = beta / InnerProduct(z, p);
		Transform(x, p, LinearIncrement(alphaK));
		Transform(&r, z, LinearIncrement(-alphaK));
		if (sqrt(InnerProduct(r, r)) <= tNorm)
			return;
	}
	THROW("Exhausted iterations in CGSolve");
}

