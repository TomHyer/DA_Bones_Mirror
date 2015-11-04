
#include "Platform.h"
#include "MatrixArithmetic.h"
#include "Strict.h"

#include "Matrix.h"
#include "Algorithms.h"
#include "Numerics.h"
#include "Functionals.h"

using std::multiplies;

namespace
{
	void MultiplyAliasFree
		(const Matrix_<>& left,
		 const Matrix_<>& right,
		 Matrix_<>* result)
	{
		assert(result != &right);
		result->Resize(left.Rows(), right.Cols());
		result->Fill(0.0);
		for (int ir = 0; ir < left.Rows(); ++ir)
		{
			auto dst = result->Row(ir);
			for (int jr = 0; jr < right.Rows(); ++jr)
				Transform(&dst, right.Row(jr), LinearIncrement(left(ir, jr)));
		}
	}

	void MultiplyAliasFree
		(const Matrix_<>& left,
		 const Vector_<>& right,
		 Vector_<>* result)
	{
		assert(result != &right);
		result->Resize(left.Rows());
		for (int ir = 0; ir < left.Rows(); ++ir)
			(*result)[ir] = InnerProduct(left.Row(ir), right);
	}

	void MultiplyAliasFree
		(const Vector_<>& left,
		 const Matrix_<>& right,
		 Vector_<>* result)
	{
		assert(result != &left);
		result->Resize(right.Cols());
		result->Fill(0.0);
		assert(left.size() == right.Rows());
		for (int ir = 0; ir < right.Rows(); ++ir)
			Transform(result, right.Row(ir), LinearIncrement(left[ir]));
	}
}	// leave local

void Matrix::Multiply
   (const Matrix_<>& left,
   const Matrix_<>& right,
   Matrix_<>* result)
{
	assert(left.Cols() == right.Rows());
	if (result == &left)
		Multiply(Matrix_<>(left), right, result);
	else if (result == &right)
		Multiply(left, Matrix_<>(right), result);
	else
		MultiplyAliasFree(left, right, result);
}

void Matrix::Multiply
   (const Matrix_<>& left,
   const Vector_<>& right,
   Vector_<>* result)
{
	assert(left.Cols() == right.size());
	if (result == &right)	// aliased
		Multiply(left, Vector_<>(right), result);
	else
		MultiplyAliasFree(left, right, result);
}

void Matrix::Multiply
   (const Vector_<>& left,
    const Matrix_<>& right,
    Vector_<>* result)
{
	assert(right.Rows() == left.size());
	if (result == &left)	// aliased
		Multiply(Vector_<>(left), right, result);
	else
		MultiplyAliasFree(left, right, result);
}

double Matrix::WeightedInnerProduct
   (const Vector_<>& left,
    const Matrix_<>& w,
    const Vector_<>& right)
{
	assert(left.size() == w.Rows());
	assert(right.size() == w.Cols());
	double retval = 0.0;
	for (int ir = 0; ir < w.Rows(); ++ir)
		retval += left[ir] * InnerProduct(w.Row(ir), right);
	return retval;
}


void Matrix::AddJSquaredToUpper
	(const Matrix_<>& a,
	 Matrix_<>* h)
{
	static const int CACHE_SIZE = 16;	// number of doubles in a cached row

	const int n = a.Rows(), nf = a.Cols();
	assert(h->Rows() == n && h->Cols() == n);

	for (int ii = 0; ii < n; ++ii)	// do one row of the output
	{
		for (int jOuter = ii; jOuter < n; jOuter += CACHE_SIZE)	// break the output into segments
		{
			auto src = a.Row(ii).begin();
			const int jStop = Min(n, jOuter + CACHE_SIZE);		// last j in this segment

			for (int kOuter = 0; kOuter < nf; kOuter += CACHE_SIZE, src += CACHE_SIZE)	// break the input (to inner_product) into segments
			{
				auto dst = h->Row(ii).begin() + jOuter;
				const int nfHere = Min(nf - kOuter, CACHE_SIZE);
				auto srcStop = src + nfHere;

				for (int jInner = jOuter; jInner < jStop; ++jInner, ++dst)
				{
					*dst = inner_product(src, srcStop, a.Row(jInner).begin() + kOuter, *dst);
				}
			}
		}
	}
}

Vector_<> Vector::L1Normalized(const Vector_<>& base)
{
	return Apply(bind1st(multiplies<double>(), 1.0 / Accumulate(base)), base);
}
Vector_<> Vector::L2Normalized(const Vector_<>& base)
{
	return Apply(bind1st(multiplies<double>(), 1.0 / sqrt(InnerProduct(base, base))), base);
}

Vector_<> Matrix::Vols
	(const Matrix_<>& cov,
	 Matrix_<>* corr)
{
	const int n = cov.Rows();
	assert(cov.Cols() == n);
	Vector_<> retval(n);
	if (corr)
		*corr = cov;
	for (int ii = 0; ii < n; ++ii)
	{
		retval[ii] = sqrt(cov(ii, ii));
		if (corr)
		{
			auto scale = std::bind2nd(std::multiplies<double>(), 1.0 / Max(DA::EPSILON, retval[ii]));
			auto r = corr->Row(ii);
			Transform(&r, scale);
			auto c = corr->Col(ii);
			Transform(&c, scale);
		}
	}
	return retval;
}

