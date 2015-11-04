
#include "Platform.h"
#include "AMC.h"
#include "Strict.h"

#include <list>
#include "Matrix.h"

namespace
{
	struct ObservableLess_
	{
		const Matrix_<>& obs_;
		const int iObs_;
		ObservableLess_(const Matrix_<>& obs, int io) : obs_(obs), iObs_(io) {}
		bool operator()(int lhs, int rhs) const
		{
			return obs_(iObs_, lhs) < obs_(iObs_, rhs);
		}
	};

	void Partition
		(const Matrix_<>& observables,
		 const Vector_<int>& n_bundles,
		 bool bundle_first,
		 Vector_<int>* key,
		 std::list<int>* breaks)
	{
		const int nPaths = observables.Cols();
		*key = Vector::UpTo(nPaths);   // [0, nPaths)

		breaks->clear();
		breaks->push_back(0);
		breaks->push_back(nPaths);
		const int minD = bundle_first ? 0 : 1;
		for (int d = observables.Rows() - 1; d >= minD; --d)
		{
         ObservableLess_ compare(observables, d);
			int from = breaks->front();
			for (auto p = Next(breaks->begin()); p != breaks->end(); from = *p++)
			{
				std::sort(&(*key)[from], &(*key)[*p], compare);
				const int size = *p - from;
				const int nb = Min(size, n_bundles[d]);
				for (int j = 1; j < nb; ++j)
					breaks->insert(p, from + (size * j) / nb);
			}
		}
	}
}