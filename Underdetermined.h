
// underdetermined search engine

#pragma once

#include "Vectors.h"

namespace Sparse
{
	class Square_;
   class SymmetricDecomposition_;
}

/*IF--------------------------------------------------------------------------
settings UnderdeterminedControls
	controls for underdetermined search
&members
maxEvaluations is integer
	Give up after this many point evaluations
maxRestarts is integer
	Give up after this many gradient calculations
maxBacktrackTries is integer default 5
	Iteration limit during backtracking linesearch
restartTolerance is number default 0.4
	Restart when k_min is above this limit
backtrackTolerance is number default 0.1
   Don't start backtracking until k_min exceeds this
maxBacktrack is number default 0.8
   Never backtrack more than this fraction of a step
&conditions
maxEvaluations_ > 0
maxRestarts_ > 0
restartTolerance_ >= 0.0 && restartTolerance_ <= 1.0
maxBacktrack_ > backtrackTolerance_ && maxBacktrack_ < 1.0
-IF-------------------------------------------------------------------------*/

#include "MG_UnderdeterminedControls_object.h"

namespace Underdetermined
{
	typedef UnderdeterminedControls_ Controls_;

   // Jacobian is indexed as J[i_f][i_x] -- J is long and low, J^T is tall and thin
	class Jacobian_
	{
	public:
		virtual ~Jacobian_();

		virtual int Rows() const = 0;
		virtual int Columns() const = 0;

		virtual void DivideRows(const Vector_<>& tol) = 0; 
      virtual Vector_<> MultiplyLeft(const Vector_<>& dx) const = 0;
      virtual Vector_<> MultiplyRight(const Vector_<>& t) const = 0;
      virtual void QForm
			(const Sparse::SymmetricDecomposition_& w,
			 SquareMatrix_<>* form) const = 0;
		virtual void SecantUpdate
			(const Vector_<>& dx, const Vector_<>& df) = 0;
	};

	class Function_
	{
		virtual double BumpSize() const;
		virtual void FFast(const Vector_<>& x, Vector_<>* f) const { *f = F(x);	}
	public:
		virtual ~Function_();
		virtual Vector_<> F(const Vector_<>& x) const = 0;

		// many possible Jacobian implementations
			// default is bump-and-grind using FFast (which might just call F) and BumpSize()
				// but Jacobian_* Gradient is called first, and the result is used if non-null
			// derived class can override dense-Jacobian calculator
			// or can supply a nontrivial implementation of the first function returning a custom Jacobian_ object
		virtual Jacobian_* Gradient
			(const Vector_<>& x,
			 const Vector_<>& f)
		const 
		{
			return nullptr;
		}
		virtual void Gradient
			(const Vector_<>& x,
			 const Vector_<>& f, 
			 Matrix_<>* j) 
		const;
	};

	Vector_<> Find
		(const Function_& func,
		 const Vector_<>& guess,
		 const Vector_<>& tol,
		 const Sparse::SymmetricDecomposition_& w,   // already decomposed
		 const Controls_& controls,
		 Matrix_<>* eff_j_inv = nullptr);

   Vector_<> Approximate
      (const Function_& func_in,
       const Vector_<>& guess,
       const Vector_<>& func_tol,
       double fit_tol,
       const Sparse::Square_& w,
       const Controls_& controls);
}

