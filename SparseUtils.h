
// utility functions for manipulating sparse matrices

#pragma once

#include "Sparse.h"

namespace Sparse
{
	inline void AddBinomial
		(Square_* dst, double scale, int i, int j, double w_i, double w_j)
	{
		dst->Add(i, i, scale * w_i * w_i);
		dst->Add(i, j, scale * w_i * w_j);
		dst->Add(j, i, scale * w_i * w_j);
		dst->Add(j, j, scale * w_j * w_j);
	}
	inline void AddCoupling
		(Square_* dst, int i, int j, double amount)
	{
		AddBinomial(dst, amount, i, j, 1.0, -1.0);
	}
}