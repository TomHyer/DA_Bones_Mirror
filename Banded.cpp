
#include "Platform.h"
#include "Banded.h"
#include "Strict.h"

#include "Sparse.h"
#include "Algorithms.h"
#include "Numerics.h"
#include "Exceptions.h"
#include "SquareMatrix.h"

namespace
{
   static const double ZERO = 0.0;  // used for out-of-band matrix elements

   void TriMultiply
		(const Vector_<>& x,
		 const Vector_<>& diag,
		 const Vector_<>& above,
		 const Vector_<>& below,
		 Vector_<>* r)
	{
		assert(x.size() == diag.size());
		r->Resize(x.size());
		Transform(x, diag, std::multiplies<double>(), r);
		auto pr = r->begin();	// can't declare in the for loop because type is different (this is a non-const iterator)
		for (auto px = x.begin() + 1, pa = above.begin(); pa != above.end(); ++px, ++pa, ++pr)
			*pr += *px * *pa;
		pr = r->begin() + 1;
		for (auto px = x.begin(), pb = below.begin(); pb != below.end(); ++px, ++pb, ++pr)
			*pr += *px * *pb;
	}

   Vector_<> TridagBetaInverse
      (const Vector_<>& diag,
       const Vector_<>& above,
       const Vector_<>& below)
   {
      const int n = diag.size();
      Vector_<> retval(n);
      double gammaA = 0.0;
      for (int j = 0;; ++j)
      {
         const double beta = diag[j] - gammaA;
         REQUIRE(!IsZero(beta), "Tridiagonal decomposition failed");
         retval[j] = 1.0 / beta;
         if (j == n - 1)
            return retval;
         gammaA = above[j] * retval[j] * below[j];
      }
   }

   void TriSolve
      (const Vector_<>& b,
       const Vector_<>& diag,
       const Vector_<>& above,
       const Vector_<>& below,
       const Vector_<>& beta_inv,
       Vector_<>* x)
   {
      const int n = diag.size();
      assert(b.size() == n);
      x->Resize(n);
      (*x)[0] = b[0] * beta_inv[0];
      for (int j = 1; j < n; ++j)
         (*x)[j] = (b[j] - above[j - 1] * (*x)[j - 1]) * beta_inv[j];
      for (int j = n - 1; j > 0; --j)
         (*x)[j - 1] -= below[j - 1] * beta_inv[j - 1] * (*x)[j];
   }

   struct TriDecomp_ : SquareMatrixDecomposition_
   {
      Vector_<> diag_, above_, below_;
      Vector_<> betaInv_;
      TriDecomp_(const Vector_<>& diag,
         const Vector_<>& above,
         const Vector_<>& below)
      : diag_(diag), above_(above), below_(below), betaInv_(TridagBetaInverse(diag, above, below))
      {  }

      int Size() const override { return diag_.size(); }
      void XMultiplyLeft_af(const Vector_<>& x, Vector_<>* b) const override
      {
         assert(x.size() == Size());
         TriMultiply(x, diag_, above_, below_, b);
      }
      void XMultiplyRight_af(const Vector_<>& x, Vector_<>* b) const override
      {
         assert(x.size() == Size());
         TriMultiply(x, diag_, below_, above_, b);
      }
      void XSolveLeft_af(const Vector_<>& b, Vector_<>* x) const override
      {
         assert(b.size() == Size());
         TriSolve(b, diag_, below_, above_, betaInv_, x);
      }
      void XSolveRight_af(const Vector_<>& b, Vector_<>* x) const override
      {
         assert(b.size() == Size());
         TriSolve(b, diag_, above_, below_, betaInv_, x);
      }
   };

   // simplified version for symmetric case
   struct TriDecompSymm_ : Sparse::SymmetricDecomposition_
   {
      Vector_<> diag_, above_;
      Vector_<> betaInv_;
      TriDecompSymm_(const Vector_<>& diag, const Vector_<>& above)
      : diag_(diag), above_(above), betaInv_(TridagBetaInverse(diag, above, above))
      {  }

      int Size() const override { return diag_.size(); }
      void XMultiply_af(const Vector_<>& x, Vector_<>* b)
         const override
      {
         assert(x.size() == Size());
         TriMultiply(x, diag_, above_, above_, b);
      }
      void XSolve_af(const Vector_<>& b, Vector_<>* x) const override
      {
         assert(b.size() == Size());
         TriSolve(b, diag_, above_, above_, betaInv_, x);
      }
      Vector_<>::const_iterator MakeCorrelated
         (Vector_<>::const_iterator iid_begin,
          Vector_<>* correlated)
      const override
      {
         THROW("Tridiagonal correlation matrices are not supported");
      }
   };

   template<class V_> auto TridiagAt(V_& diag, V_& above, V_& below, int i_row, int i_col)
      -> decltype(&diag[0])
   {
      if (abs(i_row - i_col) > 1)
         return nullptr;
      if (i_row == i_col)
         return &diag[i_row];
      return i_row > i_col ? &below[i_col] : &above[i_row];
   }

   class Tridiagonal_ : public Sparse::Square_
	{
		Vector_<> diag_, above_, below_;
	public:
      Tridiagonal_(int size) : diag_(size, 0.0), above_(size - 1, 0.0), below_(size - 1, 0.0) {}
		int Size() const override { return diag_.size(); }
		bool IsSymmetric() const override { return above_ == below_; }

      double* At(int i_row, int i_col)
      {
         return TridiagAt(diag_, above_, below_, i_row, i_col);
      }
      const double& operator()(int i_row, int i_col) const override
      {
         const double* temp = TridiagAt(diag_, above_, below_, i_row, i_col);
         return temp ? *temp : ZERO;
      }
	  void Set(int i_row, int i_col, double val) override
	  {
		  double* dst = At(i_row, i_col);
		  REQUIRE(dst, "Out-of-band write to tridiagonal");
		  *dst = val;
	  }
	  void Add(int i_row, int i_col, double inc) override
	  {
		  double* dst = At(i_row, i_col);
		  REQUIRE(dst, "Out-of-band write to tridiagonal");
		  *dst += inc;
	  }

      void MultiplyLeft(const Vector_<>& x, Vector_<>* b) const override { TriMultiply(x, diag_, above_, below_, b); }
      void MultiplyRight(const Vector_<>& x, Vector_<>* b) const override { TriMultiply(x, diag_, below_, above_, b); }
      SquareMatrixDecomposition_* Decompose() const override 
      { 
         if (IsSymmetric())
            return new TriDecompSymm_(diag_, above_);
         return new TriDecomp_(diag_, above_, below_); 
      }
   };

	//------------------------------------------------------------------------
	// more general band diagonals
	// band diagonals are stored as in Numerical Recipes:  columns below diagonal, then diagonal, then above diagonal
	struct BandElements_
	{
		Matrix_<> store_;
		const Matrix_<>& view_;
		int nBelow_;
		
		BandElements_(int size, int n_below, int n_above)
			:
		store_(size, 1 + n_below + n_above),
		view_(store_),
		nBelow_(n_below)
		{
			store_.Fill(0.0);
		}
		// also can borrow an existing matrix, in which case store_ will be empty
		BandElements_(const Matrix_<>& vals, int n_below)
			:
		view_(vals),
		nBelow_(n_below)
		{	}

		const double& operator()(int row, int col) const
		{
			const int myCol = (col - row) + nBelow_;
			if (myCol >= 0 && myCol < view_.Cols())
				return view_(row, myCol);
			return ZERO;
		}
		// only call this function to write in the nonzero region -- it will throw otherwise
		double& At(int row, int col)
		{
			REQUIRE(!store_.Empty(), "Can't write to view-only band elements");
			const int myCol = (col - row) + nBelow_;
			REQUIRE(myCol >= 0 && myCol < store_.Cols(), "Index outside diagonal band");
			return store_(row, myCol);
		}
	};

	// first, some free functions supporting implementation

	// left-multiplication
	template<bool transpose> void BandedMultiply
		(const BandElements_& vals,
		 const Vector_<>& x,
		 Vector_<>* b)
	{
		assert(b != &x);	// no aliasing here
		const int n = x.size();
		assert(vals.view_.Rows() == n);
		b->Resize(n);
		b->Fill(0.0);
		for (int ii = 0; ii < n; ++ii)
		{
			const int jStop = Min(n, ii + vals.view_.Cols() - vals.nBelow_);
			for (int jj = Max(0, ii - vals.nBelow_); jj < jStop; ++jj)
				(*b)[transpose ? jj : ii] += x[transpose ? ii : jj] * vals(ii, jj);
		}
	}

	// this works even if x == &b
	void BandedLSolve
		(const BandElements_& vals,
		 const Vector_<>& b,
		 Vector_<>* x)
	{
		assert(vals.view_.Cols() == vals.nBelow_ + 1);
		const int n = b.size();
		assert(vals.view_.Rows() == n);
		x->Resize(n);
		for (int ii = 0; ii < n; ++ii)
		{
			double residual = b[ii];
			for (int jj = Max(0, ii - vals.nBelow_); jj < ii; ++jj)
				residual -= (*x)[jj] * vals(ii, jj);
			REQUIRE(!IsZero(vals(ii, ii)), "Overflow in banded L-solve");
			(*x)[ii] = residual / vals(ii, ii);
		}
	}
	// this works even if x == &b
	void BandedLTransposeSolve
		(const BandElements_& vals,
		 const Vector_<>& b,
		 Vector_<>* x)
	{
		assert(vals.view_.Cols() == vals.nBelow_ + 1);
		const int n = b.size();
		assert(vals.view_.Rows() == n);
		x->Resize(n);
		for (int ii = n - 1; ii >= 0; --ii)
		{
			double residual = b[ii];
			for (int jj = Max(n - 1, ii + vals.nBelow_); jj > ii; --jj)
				residual -= (*x)[jj] * vals(jj, ii);	// reversal of indices of vals is the Transpose in action
			REQUIRE(!IsZero(vals(ii, ii)), "Overflow in banded L-solve");
			(*x)[ii] = residual / vals(ii, ii);
		}
	}

	// decomposition
	class BandedCholesky_ : public Sparse::SymmetricDecomposition_
	{
		BandElements_ vals_;

	public:
		// construct by Cholesky-decomposing an input
		BandedCholesky_(const BandElements_& llt)
			:
		vals_(llt.view_.Rows(), llt.nBelow_, 0)
		{
			static const double SMALL = 1.0e-11;
			NOTE("Performing banded Cholesky decomposition");
			assert(llt.view_.Cols() == 2 * llt.nBelow_ + 1);
			const int n = llt.view_.Rows();
			for (int ii = 0; ii < n; ++ii)
			{
				NOTICE(ii);
				const int iMin = Max(0, ii - llt.nBelow_);
				for (int jj = iMin; jj <= ii; ++jj)
				{
					double residual = llt(ii, jj);
					for (int kk = iMin; kk < jj; ++kk)
						residual -= vals_(ii, kk) * vals_(jj, kk);
					if (jj < ii)
					{
						if (IsZero(residual))
							vals_.At(ii, jj) = 0.0;
						else
						{
							REQUIRE(!IsZero(vals_(jj, jj)), "Overflow");
							vals_.At(ii, jj) = residual / vals_(jj, jj);
						}
					}
					else
					{
						REQUIRE(residual > -SMALL, "Non-positive-definite matrix");
						vals_.At(ii, ii) = sqrt(residual);
					}
				}
			}
		}

		int Size() const override { return vals_.view_.Rows(); }

		void XMultiply_af
			(const Vector_<>& x,
			 Vector_<>* b)
		const override
		{
			// our implementation of Multiply requires a spare vector
			Vector_<> temp;
			BandedMultiply<true>(vals_, x, &temp);
			BandedMultiply<false>(vals_, temp, b);
		}

		void XSolve_af
			(const Vector_<>& b,
			 Vector_<>* x)
		const override
		{
			BandedLSolve(vals_, b, x);
			BandedLTransposeSolve(vals_, *x, x);	
		}

		Vector_<>::const_iterator MakeCorrelated
			(Vector_<>::const_iterator iid_begin,
			 Vector_<>* correlated)
		const override
		{
			const int n = Size();
			correlated->Resize(n);
			correlated->Fill(0.0);
			for (int ii = 0; ii < n; ++ii, ++iid_begin)
			{
				for (int jj = Min(n - 1, ii + vals_.nBelow_); jj >= ii; --jj)
					(*correlated)[jj] += vals_(jj, ii) * *iid_begin;
			}
			return iid_begin;
		}

		void QForm(const Matrix_<>& j, SquareMatrix_<>* form) const override
		{
			assert(j.Cols() == Size());
			Vector_<Vector_<> > tm(j.Rows());
			// compute L^{-1} J
			for (int ii = 0; ii < j.Rows(); ++ii)
			{
				tm[ii].Resize(Size());
				Copy(j[ii], &tm[ii]);
				BandedLSolve(vals_, tm[ii], &tm[ii]); // in-place
			}
			// compute result
			form->Resize(j.Rows());
			for (int io = 0; io < j.Rows(); ++io)
				for (int k = 0; k <= io; ++k)
					(*form)(io, k) = (*form)(k, io) = InnerProduct(tm[io], tm[k]);
		}
	};

	class Banded_ : public Sparse::Square_
	{
		BandElements_ vals_;
	public:
		Banded_(int size, int n_above, int n_below) : vals_(size, n_above, n_below) {}

		int Size() const override { return vals_.view_.Rows(); }
		void MultiplyLeft
			(const Vector_<>& x,
			 Vector_<>* b)
		const override
		{
			BandedMultiply<false>(vals_, x, b);
		}
		virtual void MultiplyRight
			(const Vector_<>& x,
			 Vector_<>* b)
		const override
		{
			BandedMultiply<true>(vals_, x, b);
		}

		bool IsSymmetric() const override
		{
			// brute-force check
			const int n = Size();
			const int width = Max(vals_.nBelow_, vals_.view_.Cols() - vals_.nBelow_ - 1);
			for (int ii = 0; ii < n; ++ii)
			{
				for (int jj = Max(0, ii - width); jj <= Max(n - 1, ii + width); ++ii)
				if (!IsZero(vals_(ii, jj) - vals_(jj, ii)))
					return false;
			}
			return true;
		}
		SquareMatrixDecomposition_* Decompose() const override
		{
			REQUIRE(IsSymmetric(), "Cholesky decomposition requires a symmetric matrix");
			// POSTPONED -- return banded LU decomposition
			return new BandedCholesky_(vals_);
		}

		const double& operator()(int row, int col) const override { return vals_(row, col); }
		void Set(int row, int col, double val) override { vals_.At(row, col) = val; }
		void Add(int row, int col, double val) override { vals_.At(row, col) += val; }
	};
}	// leave local

Sparse::Square_* Sparse::NewBandDiagonal(int size, int n_above, int n_below)
{
   assert(size > 0);
   if (n_above <= 1 && n_below <= 1)
      return new Tridiagonal_(size);
	return new Banded_(size, n_above, n_below);
}

//----------------------------------------------------------------------------

namespace
{
	Vector_<> PadAtFront(const Vector_<>& src, int size)
	{
		assert(src.size() <= size);
		if (src.size() >= size)
			return src;
		return Concatenate(Vector_<>(size - src.size(), 0.0), src);
	}
}

LowerBandAccumulator_::LowerBandAccumulator_(int size, int n_below)
	:
vals_(size, n_below + 1)
{	
	vals_.Fill(0.0);
}

void LowerBandAccumulator_::Add(const Vector_<>& v_in, int offset)
{
	NOTE("In LowerBandAccumulator_::Add");
	REQUIRE(v_in.size() <= vals_.Cols(), "Too many nonzero elements in V");
	int iRow = v_in.size() + offset;
	REQUIRE(iRow < vals_.Rows(), "V is too large");

	Vector_<> v;
	while (iRow >= 0)
	{
		auto& row = vals_.Row(iRow);
		if (AllOf(row, IsZero))
		{	// row is empty, can just write in v
			std::copy(v.begin(), v.end(), row.end() - v.size());
			break;
		}
		if (AllOf(v, IsZero))
			break;
		// need to rotate it in
		if (v.empty())
			v = PadAtFront(v_in, Min(iRow + 1, vals_.Cols()));	// make sure v has the right size
		const double r = sqrt(Square(v.back()) + Square(row.back()));
		const double c = row.back() / r;
		const double s = v.back() / r;
		
		auto pr = row.begin() + (row.size() - v.size());
		for (auto pv = v.begin(); pv != v.end(); ++pv, ++pr)
		{	// rotation
			const double save = *pv;
			*pv = c * *pv - s * *pr;
			*pr = c * *pr + s * save;
		}
		assert(IsZero(v.back()));	// we just made it so
		// now the residue has to be rotated into the previous row
		--iRow;
		if (iRow >= v.size())	// unfortunately we need to push_front
			std::rotate(v.begin(), v.end() - 1, v.end());
		else
			v.pop_back();
	}
}

void LowerBandAccumulator_::SolveLeft(const Vector_<>& b, Vector_<>* x) const
{
	BandedLSolve(BandElements_(vals_, vals_.Cols() - 1), b, x);
}
void LowerBandAccumulator_::SolveRight(const Vector_<>& b, Vector_<>* x) const
{
	BandedLTransposeSolve(BandElements_(vals_, vals_.Cols() - 1), b, x);
}

