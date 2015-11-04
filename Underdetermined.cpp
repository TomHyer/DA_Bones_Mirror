
#include "Platform.h"
#include "Underdetermined.h"
#include "Strict.h"

#include "Functionals.h"
#include "Numerics.h"
#include "Exceptions.h"
#include "SquareMatrix.h"
#include "MatrixArithmetic.h"
#include "Cholesky.h"
#include "Sparse.h"
#include "BCG.h"

using namespace Underdetermined;

#include "MG_UnderdeterminedControls_object.inc"

Underdetermined::Function_::~Function_()
{  }

Underdetermined::Jacobian_::~Jacobian_()
{  }

double Underdetermined::Function_::BumpSize() const
{
   return 1.0e-4;
}

void Underdetermined::Function_::Gradient
   (const Vector_<>& x,
    const Vector_<>& f,
    Matrix_<>* j)
const
{
   //   virtual void FFast(const Vector_<>& x, Vector_<>* f) const { *f = F(x); }
   Vector_<> fBase, fBumped;
   FFast(x, &fBase);
   Vector_<> xBumped(x);
   const double dx = BumpSize();
   auto scale = bind2nd(std::multiplies<double>(), 1.0 / dx);
   const int nx = x.size();
   j->Resize(f.size(), nx);
   for (int ix = 0; ix < nx; ++ix)
   {
      xBumped[ix] += dx;
      FFast(xBumped, &fBumped);
      fBumped -= fBase;
	  auto col = j->Col(ix);
      Transform(fBumped, scale, &col);
      xBumped[ix] = x[ix];
   }
}

namespace
{
   // ephemeral struct -- J is stored externally to avoid copying it
   struct XJDense_ : Jacobian_
   {
      Matrix_<>& j_;
      XJDense_(Matrix_<>& j) : j_(j) {}

      int Rows() const override { return j_.Rows(); }
      int Columns() const override { return j_.Cols(); }

      void DivideRows(const Vector_<>& tol) override
      {
		  for (int ii = 0; ii < j_.Rows(); ++ii)
		  {
			  auto row = j_.Row(ii);
			  Transform(&row, bind2nd(std::multiplies<double>(), 1.0 / tol[ii]));
		  }
      }

      Vector_<> MultiplyRight(const Vector_<>& t) const override
      {
         Vector_<> retval;
         Matrix::Multiply(t, j_, &retval);
         return retval;
      }
      Vector_<> MultiplyLeft(const Vector_<>& dx) const override
      {
         Vector_<> retval;
         Matrix::Multiply(j_, dx, &retval);
         return retval;
      }

      void QForm(const Sparse::SymmetricDecomposition_& w, SquareMatrix_<>* form) const override
      {
         w.QForm(j_, form);
      }

      void SecantUpdate(const Vector_<>& dx, const Vector_<>& df) override
      {
         const int nf = df.size();
         const double x2 = InnerProduct(dx, dx);
         for (int ii = 0; ii < nf; ++ii)
         {
			 auto row = j_.Row(ii);
            const double excess = df[ii] - InnerProduct(dx, row);
            Transform(&row, dx, LinearIncrement(excess / x2));
         }
      }
   };

   struct XScaledFunc_
   {
      const Vector_<>& tol_;
      const Function_& func_;
      int nEvals_, nRestarts_;
      Matrix_<> jDense_;   // provides memory, if needed, underpinning jacobian structure

      XScaledFunc_(const Vector_<>& tol, const Function_& func, const Controls_& controls) : tol_(tol), func_(func), nEvals_(controls.maxEvaluations_), nRestarts_(controls.maxRestarts_) {}

      Vector_<> F(const Vector_<>& x)
      {
         REQUIRE(nEvals_-- > 0, "Exhausted function evaluations in underdetermined search");
         Vector_<> retval = func_.F(x);
         Transform(&retval, tol_, std::divides<double>());
         return retval;
      }

      Jacobian_* J(const Vector_<>& x, const Vector_<>& f)
      {
         REQUIRE(nRestarts_-- > 0, "Exhausted gradient evaluations in underdetermined search");
         if (auto sparse = func_.Gradient(x, f))
         {
            sparse->DivideRows(tol_);
            return sparse;
         }
         // have to set up dense J
         func_.Gradient(x, f, &jDense_);
         std::unique_ptr<XJDense_> retval(new XJDense_(jDense_));
         retval->DivideRows(tol_);
         return retval.release();
      }
   };

   void QPStep
      (const Vector_<>& f,
       const Jacobian_& j,
       const Sparse::SymmetricDecomposition_& w,
       SquareMatrix_<>* q, // used as workspace, invalid on output
       Vector_<>* s)
   {
      // W^{-1} J^T (J W^{-1} J^T)^{-1} f
      j.QForm(w, q);
      Vector_<Vector_<>> qf(1, f);
      qf[0] *= -1.0;
      CholeskySolve(q, &qf);  // q is now ruined, its lower triangle replaced with the Cholesky decomposition
      Vector_<> ws = j.MultiplyRight(qf[0]);
      w.Solve(ws, s);
   }

   // Here is a "sparse" matrix (compactly represented, anyway) of the form W + J^T J
   // this form arises in approximate underdetermined search
   class XPenaltyWeight_ : public Sparse::Square_
   {
      const Sparse::Square_& W_; // must be symmetric
      const Underdetermined::Jacobian_& J_;
      double jWeight_;

      int Size() const override { return W_.Size(); }
      virtual void MultiplyLeft
         (const Vector_<>& x, Vector_<>* b) 
      const override
      {
         W_.MultiplyLeft(x, b);
         const Vector_<> Js = J_.MultiplyLeft(x);
         Transform(b, J_.MultiplyRight(Js), LinearIncrement(jWeight_));
      }

      virtual void MultiplyRight(const Vector_<>& x, Vector_<>* b) const override { MultiplyLeft(x, b); }
      bool IsSymmetric() const override { assert(W_.IsSymmetric()); return true; }
      Sparse::SymmetricDecomposition_* Decompose() const override; // see below
      const double& operator()(int i_row, int i_col) const override { THROW("Penalty weight element access is not supported"); } // too hard
      void Set(int i_row, int i_col, double val) override { THROW("Penalty weight element setting is not possible"); }
   public:
      XPenaltyWeight_(const Sparse::Square_& W, const Underdetermined::Jacobian_& J, double j_weight) : W_(W), J_(J), jWeight_(j_weight) {}
   };

   // decomposition based on BCG, with ephemeral class holding references
   class XDecompByCG_ : public Sparse::SymmetricDecomposition_
   {
      const Sparse::Square_& A_;  // must be symmetric
   public:
      XDecompByCG_(const Sparse::Square_& A) : A_(A) {}

      void XMultiply_af(const Vector_<>& x, Vector_<>* b) const override { A_.MultiplyLeft(x, b); }
      Vector_<>::const_iterator MakeCorrelated(Vector_<>::const_iterator, Vector_<>*) const override { THROW("Correlation by penalty weight is not supported"); }
      int Size() const override { return A_.Size(); }

      virtual void XSolve_af
         (const Vector_<>& b,
          Vector_<>* x)
      const override
      {
         static const double TOL_REL = 0.00001;
         const double tol_abs = sqrt(b.size());
         const int iterations = 200 + AsInt(8.0 * sqrt(b.size()));
         x->Resize(b.size());
         x->Fill(0.0);    // no real basis for a guess
         Sparse::CGSolve(A_, b, TOL_REL, tol_abs, iterations, x);
      }
   };

   Sparse::SymmetricDecomposition_* XPenaltyWeight_::Decompose() const
   {
      return new XDecompByCG_(*this);
   }

   Vector_<> ApproxQPStep
      (const Vector_<>& x0,
       const Vector_<>& x,
       const Vector_<>& f,
       const Underdetermined::Jacobian_& j,
       double j_weight,
       const Sparse::Square_& w)
   {
      XPenaltyWeight_ WJJ(w, j, j_weight);
      scoped_ptr<Sparse::SymmetricDecomposition_> decomp(WJJ.DecomposeSymmetric());
      Vector_<> rhs, retval;
      w.MultiplyLeft(Apply(std::minus<double>(), x0, x), &rhs);
      Transform(&rhs, j.MultiplyRight(f), LinearIncrement(-j_weight));
      decomp->Solve(rhs, &retval);
      return retval;
   }
}  // leave local


Vector_<> Underdetermined::Find
   (const Function_& func_in,
    const Vector_<>& guess,
    const Vector_<>& tol,
    const Sparse::SymmetricDecomposition_& w,
    const Controls_& controls,
    Matrix_<>* eff_j_inv)
{
   // set up the wrapper through which we will call the function
   XScaledFunc_ func(tol, func_in, controls);
   Vector_<> xOld(guess);
   Vector_<> fOld = func.F(xOld);
   std::unique_ptr<Jacobian_> j;
   Vector_<> xNew(xOld.size()), s(xOld.size());
   SquareMatrix_<> q;

   bool approxJ = false, restart = true;
   for (;;)
   {
      if (restart)
      {
         j.reset(func.J(xOld, fOld));
         approxJ = false;
         restart = false;
      }
      QPStep(fOld, *j, w, &q, &s);
      // backtracking linesearch
      bool tookStep = false;
      for (int iBacktrack = 0; iBacktrack < controls.maxBacktrackTries_; ++iBacktrack)
      {
         Transform(xOld, s, std::plus<double>(), &xNew);
         Vector_<> fNew = func.F(xNew);
         if (*MaxElement(fNew) < 1.0 && *MinElement(fNew) > -1.0)
            return xNew;

         const double oldOld = InnerProduct(fOld, fOld);
         const double oldNew = InnerProduct(fOld, fNew);
         const double newNew = InnerProduct(fNew, fNew);
         // \tilde f^2 = oldOld * k^2 + 2 * oldNew * k * (1-k) + newNew * (1-k)^2
         // its k-derivative is oldOld * 2 * k + oldNew * (1 - 2*k) + 2 * newNew * (k-1)
         double kMin = (newNew - 0.5 * oldNew) / (newNew - oldNew + oldOld);
         if (kMin < controls.backtrackTolerance_)
         {
            if (!restart)
            {
               j->SecantUpdate(s, Apply(std::minus<double>(), fNew, fOld));
               approxJ = true;
            }
            xOld = xNew;
            fOld = fNew;
            tookStep = true;
            break;
         }
         if (kMin > controls.restartTolerance_)
            restart = true;
         double k = Min(controls.maxBacktrack_, Min(kMin, 2 * (kMin - controls.backtrackTolerance_)));
         assert(k > 0.0);
         s *= 1.0 - k;
      }  // end backtracking loop
      REQUIRE(tookStep || approxJ, "Could not find a descent direction in underdetermined search");
   }
}

Vector_<> Underdetermined::Approximate
   (const Function_& func_in,
    const Vector_<>& guess,
    const Vector_<>& func_tol,  // accuracy of function evaluation
    double fit_tol,        // accuracy of appproximate fit
    const Sparse::Square_& w,
    const Controls_& controls)
{
   // set up the wrapper through which we will call the function
   XScaledFunc_ func(func_tol, func_in, controls);
   Vector_<> xOld(guess);
   Vector_<> fOld = func.F(xOld);
   std::unique_ptr<Jacobian_> j;
   Vector_<> xNew(xOld.size());
   SquareMatrix_<> q;
   const double jWeight = InnerProduct(func_tol, func_tol) / Square(fit_tol);

   for (int ie = 3, ticker = 0; ie < controls.maxEvaluations_; ++ie, ticker -= controls.maxRestarts_)
   {
      if (ticker <= 0)
      {
         j.reset(func.J(xOld, fOld));
         ticker = controls.maxEvaluations_;
         ++ie; // we used up an evaluation too
      }
      Vector_<> s = ApproxQPStep(guess, xOld, fOld, *j, jWeight, w);
      // const double sNorm = InnerProduct(s, s); POSTPONED -- stop early when step is very small
      Transform(xOld, s, std::plus<double>(), &xNew);
      Vector_<> fNew = func.F(xNew);
      if (ticker > controls.maxRestarts_) // we are not about to restart
         j->SecantUpdate(s, Apply(std::minus<double>(), fNew, fOld));
      xOld = xNew;
      fOld = fNew;
   }
   return xOld;
}

