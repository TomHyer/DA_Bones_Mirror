
#include "Platform.h"
#include "Vectors.h"
#include "Strict.h"

Vector_<int> Vector::UpTo(int n)
{
	Vector_<int> retval(n);
	for (int ii = 0; ii < n; ++ii)
		retval[ii] = ii;
	return retval;
}
