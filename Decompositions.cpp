
#include "Platform.h"
#include "Decompositions.h"
#include "Strict.h"

#define COPY_ALIAS_AND_FORWARD(cname, func, imp)	\
void cname::func									\
	(const Vector_<>& x,							\
	 Vector_<>* b)									\
const												\
{													\
	if (&x == b)									\
		imp(Vector_<>(x), b);						\
	else											\
		imp(x, b);									\
}

COPY_ALIAS_AND_FORWARD(SquareMatrixDecomposition_, SolveLeft, XSolveLeft_af)
COPY_ALIAS_AND_FORWARD(SquareMatrixDecomposition_, SolveRight, XSolveRight_af)
COPY_ALIAS_AND_FORWARD(SymmetricMatrixDecomposition_, Multiply, XMultiply_af)
COPY_ALIAS_AND_FORWARD(SymmetricMatrixDecomposition_, Solve, XSolve_af)


