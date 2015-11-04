
#include "Platform.h"
#include "Eispack.h"
#include "Strict.h"

#include "SquareMatrix.h"
#include "Exceptions.h"
#include "Decompositions.h"
#include "Numerics.h"
#include "Functionals.h"

namespace
{
   double Pythag(double a, double b)
   {
      return sqrt(Square(a) + Square(b));
   }
   double BetterPythag(double a, double b)
   {
      double p = Max(fabs(a), fabs(b));
      if (p == 0.0)
         return 0.0;
      double r = Square(Min(fabs(a), fabs(b)) / p);
      for (;;)
      {
         const double t = 4.0 + r;
         if (t == 4.0)
            return p;
         const double s = r / t;
         const double u = 1.0 + 2.0 * s;
         p *= u;
         r *= Square(s / u);
      }
   }

   inline double DSign(double r, double p)
   {
      return p * r > 0.0 ? r : -r;
   }

   // throughout this file, double-letter integer indices are 0-offset (reduced by 1 from their Fortran origins), while single-letter indices have the same numerical value as in the Fortran code
   // e.g. n is not changed, but ii is

   void TQL2
      (Vector_<>* d,    // diagonals of tridiagonal matrix -- replaced with eigenvalues in descrnding order
      Vector_<>* e,    // subdiagonal elements (in elements [1, n)) -- destroyed during analysis
      Matrix_<>* z)    // transformation matrix output from TRed2 -- replaced with eigenvectors
   {
      static const int MAX_ITERATIONS = 30;
      const int n = d->size();
      std::copy(Next(e->begin()), e->end(), e->begin()); // now e is populated in [0, n-1)

      e->back() = 0.0;

	  double f = 0.0, tst1 = 0.0;
	  for (int ll = 0; ll < n; ++ll)
      {
		 tst1 = Max(tst1, fabs((*d)[ll]) + fabs((*e)[ll]));
         int mm = ll;
         for (;;)
         {
            assert(mm < n);   // because e->back() == 0, we will always break out
            const double tst2 = tst1 + fabs((*e)[mm]);
            if (tst2 == tst1)
               break;
            ++mm;
         }
         if (mm != ll)  // therefore (*e)[ll] is significantly nonzero
         {
            for (int j = 0; ; ++j) // iteration counter
            {
               REQUIRE(j < MAX_ITERATIONS, "Exhausted iterations in TQL");
               // form shift
               const double g = (*d)[ll];
               const double p0 = ((*d)[ll + 1] - g) / (2.0 * (*e)[ll]);
               const double r = Pythag(p0, 1.0);
               (*d)[ll] = (*e)[ll] / (p0 + DSign(r, p0));
               const double dl1 = (*d)[ll + 1] = (*e)[ll] * (p0 + DSign(r, p0));
               const double h = g - (*d)[ll];
               for (int ii = ll + 2; ii < n; ++ii) 
                  (*d)[ii] -= h;
               f += h;
               // QL transformation
			   const double el1 = (*e)[ll + 1];
			   double p = (*d)[mm];
			   double c = 1.0, c2 = 1.0, s = 0.0;
			   double c3, s2;
               for (int ii = mm - 1; ii >= ll; --ii)
               {
                  c3 = c2;
                  c2 = c;
                  s2 = s;
                  const double g = c * (*e)[ii];
				  const double h = c * p;
                  const double r = Pythag(p, (*e)[ii]);
                  (*e)[ii + 1] = s * r;
                  s = (*e)[ii] / r;
                  c = p / r;
                  p = c * (*d)[ii] - s * g;
                  (*d)[ii + 1] = h + s * (c * g + s * (*d)[ii]);
                  // form eigenvector
                  for (int kk = 0; kk < n; ++kk)
                  {
					 const double h = (*z)(kk, ii + 1);
                     (*z)(kk, ii + 1) = s * (*z)(kk, ii) + c * h;
                     (*z)(kk, ii) = c * (*z)(kk, ii) - s * h;
                  }
               }
               const double p1 = -s * s2 * c3 * el1 * (*e)[ll] / dl1;
               (*e)[ll] = s * p1;
               (*d)[ll] = c * p1;
               const double tst2 = tst1 + fabs((*e)[ll]);
               if (tst2 == tst1)
                  break; // out of iterative loop
            }
         }
         (*d)[ll] += f;
      }
      // order eigenvalues and eigenvectors (this code does not follow the Fortran implementation)
      Vector_<int> keys = Vector::UpTo(n);
      Sort(&keys, [&](int ii, int jj) { return (*d)[ii] > (*d)[jj]; });
      // need the inverse of that permutation
      Vector_<int> locs(n);
      for (int ii = 0; ii < n; ++ii)
         locs[keys[ii]] = ii;

      for (int ik = 0; ik < n; ++ik)
      {
         const int jk = keys[ik];
         assert(jk >= ik);
         if (jk != ik)
         {
            std::swap((*d)[ik], (*d)[jk]);
            std::swap_ranges(z->Col(ik).begin(), z->Col(ik).end(), z->Col(jk).begin());
            keys[locs[ik]] = keys[ik];
            locs[keys[ik]] = locs[ik];
         }
      }
   }

   void TRed2
      (const Matrix_<>& a, // must be symmetric; only the lower triangle is used
       Vector_<>* d,             // will be populated with the diagonal elements of the tridiagonal reduced matrix
       Vector_<>* e,             // will be populated with the sub-diagonal elements of the reduced matrix, in elements [1, n)
       Matrix_<>* z)             // will be populated with the orthogonal transformation matrix produced in the reduction
   {
      const int n = a.Rows();
      *z = a;
      *d = Copy(a.Row(n - 1));
      e->Resize(n);

      for (int ii = n - 1; ii >= 1; --ii)
      {
         int ll = ii - 1;
         double h = 0.0, scale = 0.0;
         for (int kk = 0; kk <= ll; ++kk)
            scale += fabs((*d)[kk]);
         if (scale == 0.0)
         {
            (*e)[ii] = (*d)[ll];
            for (int jj = 0; jj <= ll; ++jj)
            {
               (*d)[jj] = (*z)(ll, jj);
               (*z)(ii, jj) = (*z)(jj, ii) = 0.0;
            }
         }
         else
         {
            for (int kk = 0; kk <= ll; ++kk)
            {
               (*d)[kk] /= scale;
               h += Square((*d)[kk]);
            }
            const double f0 = (*d)[ll];
            const double g = -DSign(sqrt(h), f0);
            (*e)[ii] = scale * g;
            h -= f0 * g;
            (*d)[ll] = f0 - g;
            // form a * u
            for (int jj = 0; jj <= ll; ++jj)
               (*e)[jj] = 0.0;
            for (int jj = 0; jj <= ll; ++jj)
            {
			   const double f = (*d)[jj];
               (*z)(jj, ii) = f;
               double g = (*e)[jj] + (*z)(jj, jj) * f;
               for (int kk = jj + 1; kk <= ll; ++kk)
               {
                  g += (*z)(kk, jj) * (*d)[kk];
                  (*e)[kk] += (*z)(kk, jj) * f;
               }
               (*e)[jj] = g;
            }
            // form p
            double f = 0.0;
            for (int jj = 0; jj <= ll; ++jj)
            {
               (*e)[jj] /= h;
               f += (*e)[jj] * (*d)[jj];
            }
            double hh = 0.5 * f / h;
            // form q
            for (int jj = 0; jj <= ll; ++jj)
               (*e)[jj] -= hh * (*d)[jj];
            // form reduced a
            for (int jj = 0; jj <= ll; ++jj)
            {
               for (int kk = jj; kk <= ll; ++kk)
				   (*z)(kk, jj) -= (*d)[jj] * (*e)[kk] + (*e)[jj] * (*d)[kk];
               (*d)[jj] = (*z)(ll, jj);
               (*z)(ii, jj) = 0.0;
            }
         }
         (*d)[ii] = h;
      }
      // accumulation of transformation matrices
      for (int ii = 1; ii < n; ++ii)
      {
         int ll = ii - 1;
         (*z)(n - 1, ll) = (*z)(ll, ll);
         (*z)(ll, ll) = 1.0;
         const double h = (*d)[ii];
         if (h != 0.0)
         {
            for (int kk = 0; kk <= ll; ++kk)
               (*d)[kk] = (*z)(kk, ii) / h;
            for (int jj = 0; jj <= ll; ++jj)
            {
               double g = 0.0;
               for (int kk = 0; kk <= ll; ++kk)
                  g += (*z)(kk, ii) * (*z)(kk, jj);
               for (int kk = 0; kk <= ll; ++kk)
                  (*z)(kk, jj) -= g * (*d)[kk];
            }
         }
         for (int kk = 0; kk <= ll; ++kk)
            (*z)(kk, ii) = 0.0;
      }
      for (int ii = 0; ii < n; ++ii)
      {
         (*d)[ii] = (*z)(n - 1, ii);
         (*z)(n - 1, ii) = 0.0;
      }
      (*z)(n - 1, n - 1) = 1.0;
      (*e)[0] = 0.0;
   }
}  // leave local

void Eispack::RS
   (const Matrix_<>& a, // the matrix to decompose -- must be symmetric
    Vector_<>* w,             // will be populated with eigenvalues
    Matrix_<>* z)             // will be populated with eigenvectors
{
   Vector_<> fv1;
   TRed2(a, w, &fv1, z);
   TQL2(w, &fv1, z);
}

//----------------------------------------------------------------------------
// wrap the raw EISPACK functionality in SymmetricMatrixDecomposition_
namespace
{
	struct EigenSystem_ : SymmetricMatrixDecomposition_
	{
		Vector_<> d_;	// of eigenvalues
		Vector_<Vector_<>> u_;	// of eigenvectors
		int Size() const override { return u_[0].size(); }
		int Rank() const override { return d_.size(); }

		void XMultiply_af
			(const Vector_<>& x,
			 Vector_<>* b)
		const override
		{
			b->Resize(x.size());
			b->Fill(0.0);
			for (int ii = 0; ii < d_.size(); ++ii)
				Transform(b, u_[ii], LinearIncrement(InnerProduct(x, u_[ii]) * d_[ii]));
		}

		void XSolve_af
			(const Vector_<>& b,
			 Vector_<>* x)
		const override
		{
			x->Resize(b.size());
			x->Fill(0.0);
			REQUIRE(Rank() == Size(), "Eigensystem is not full rank");
			for (int ii = 0; ii < d_.size(); ++ii)
			{
				REQUIRE(!IsZero(d_[ii]), "Eigensystem is singular");
				Transform(x, u_[ii], LinearIncrement(InnerProduct(b, u_[ii]) / d_[ii]));
			}
		}

		Vector_<>::const_iterator MakeCorrelated
			(Vector_<>::const_iterator iid_begin,
			 Vector_<>* correlated)
		const override
		{
			correlated->Resize(Size());
			correlated->Fill(0.0);
			for (int ii = 0; ii < d_.size(); ++ii)
			{
				REQUIRE(d_[ii] >= 0.0, "Eigensystem is not positive semidefinite");
				Transform(correlated, u_[ii], LinearIncrement(*iid_begin++ * sqrt(d_[ii])));
			}
			return iid_begin;
		}
	};
}	// leave local



SymmetricMatrixDecomposition_* NewEigenSystem
	(const Matrix_<>& a,
	 double discard_threshold,	// discard eigenmodes with eigenvalues below this
	 double error_threshold)	// error on eigenvalues below this
{
	std::unique_ptr<EigenSystem_> retval(new EigenSystem_);
	Matrix_<> z;
	Eispack::RS(a, &retval->d_, &z);
	// process eigenmodes
	NOTICE(error_threshold);
	while (!retval->d_.empty())
	{
		REQUIRE(retval->d_.back() > error_threshold, "Eigenvalue is too small");
		if (retval->d_.back() > discard_threshold)
			break;
		retval->d_.pop_back();
	}
	for (int ii = 0; ii < Max(1, retval->d_.size()); ++ii)	// don't let u be empty, even if d is, because Size() uses u
		retval->u_.push_back(Copy(z.Col(ii)));
	return retval.release();
}





