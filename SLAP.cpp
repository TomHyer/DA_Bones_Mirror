
#include "Platform.h"
#include "SLAP.h"
#include "Strict.h"

#include "Sparse.h"
#include "Algorithms.h"

namespace
{
   static const double ZERO = 0.0;

   // use templating just to control const-ness
	template<class D_, class O_> auto SLAPElement
		(D_& diag,
		 O_& off_diag,
		 int i_row,
		 int i_col)
    -> decltype(&diag[0])
	{
		if (i_row == i_col)
			return &diag[i_row];
		auto row = off_diag[i_row];
		for (auto pe = row.begin(); pe != row.end(); ++pe)
			if (pe->first == i_col)
				return &pe->second;
		return nullptr;
	}

	class SlapMatrix_ : public Sparse::Square_
	{
		Vector_<> diag_;
		Vector_<Vector_<pair<int, double> > > offDiag_;

		template<bool transpose> void XMultiply
			(const Vector_<>& x,
			 Vector_<>* b)
		const
		{
			const int n = Size();
			b->Resize(n);
			Transform(x, diag_, std::multiplies<double>(), b);
			// now add off-diagonal
			for (int ii = 0; ii < n; ++ii)
				for (const auto& l_v : offDiag_[ii])
						(*b)[transpose ? l_v.first : ii] += l_v.second * x[transpose ? ii : l_v.first];
		}

		void MultiplyLeft(const Vector_<>& x, Vector_<>* b) const override { XMultiply<false>(x, b); }
		void MultiplyRight(const Vector_<>& x, Vector_<>* b) const override { XMultiply<true>(x, b); }

		const double& operator()(int i_row, int i_col) const override
		{
			const double* temp = SLAPElement(diag_, offDiag_, i_row, i_col);
			return DEREFERENCE(temp, ZERO);
		}
		void Set(int i_row, int i_col, double val) override
		{
			if (double* dst = SLAPElement(diag_, offDiag_, i_row, i_col))
				*dst = val;
			else
				offDiag_[i_row].emplace_back(i_col, val);
		}
		void Add(int i_row, int i_col, double val) override
		{
			if (double* dst = SLAPElement(diag_, offDiag_, i_row, i_col))
				*dst += val;
			else
				offDiag_[i_row].emplace_back(i_col, val);
		}
	};
}	// leave local

