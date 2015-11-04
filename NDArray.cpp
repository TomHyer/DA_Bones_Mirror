
#include "Platform.h"
#include "NDArray.h"
#include "Strict.h"

#include "Algorithms.h"
#include "Numerics.h"
#include "Functionals.h"

Vector_<int> ArrayN::Strides(const Vector_<int>& sizes)
{
	Vector_<int> retval(sizes.size(), 1);
	for (int ii = 1; ii < sizes.size(); ++ii)
		retval[ii - 1] = retval[ii] * sizes[ii];
	return retval;
}

Vector_<pair<int, int>> ArrayN::Moves
	(const Vector_<int>& old_sizes,
	 const Vector_<int>& new_sizes)
{
	const int nd = old_sizes.size();
	REQUIRE(new_sizes.size() == nd, "Resize() can't change the dimension of an array");
	Vector_<int> oldStrides = Strides(old_sizes), newStrides = Strides(new_sizes);
	Vector_<int> mins = Apply(std::ptr_fun(Min<int>), old_sizes, new_sizes);
	Vector_<int> loc(nd, 0);
	Vector_<pair<int, int>> retval;
	for (;;)
	{
		int depth;
		for (depth = 0; depth < nd; ++depth)
		{
			if (++loc[depth] < mins[depth])
				break;
			loc[depth] = 0;
		}
		if (depth == nd)
			return retval;
		retval.emplace_back(InnerProduct(loc, oldStrides), InnerProduct(loc, newStrides));
	}
}

Cube_::Cube_()
:
ArrayN_<double>(Vector_<int>({ 0, 0, 0 }), 0.0)
{	}

Cube_::Cube_(int size_i, int size_j, int size_k) 
: 
ArrayN_<double>(Vector_<int>({ size_i, size_j, size_k })) 
{	}

void Cube_::Resize(int size_i, int size_j, int size_k)
{
	ArrayN_<double>::Resize(Vector_<int>({ size_i, size_j, size_k }));
}

