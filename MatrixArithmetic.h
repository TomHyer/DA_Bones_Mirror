	
#pragma once

namespace Matrix
{
	Vector_<> Vols
		(const Matrix_<>& cov,
		 Matrix_<>* corr = nullptr);	// this routine also works in-place, when corr==&cov

	void Multiply
		(const Matrix_<>& left,
		const Matrix_<>& right,
		Matrix_<>* result);

	void Multiply
		(const Matrix_<>& left,
		const Vector_<>& right,
		Vector_<>* result);

	void Multiply
		(const Vector_<>& left,
		const Matrix_<>& right,
		Vector_<>* result);

	double WeightedInnerProduct
		(const Vector_<>& left,
		const Matrix_<double>& w,
		const Vector_<>& right);

	void AddJSquaredToUpper
		(const Matrix_<>& j,
		 Matrix_<>* h);
}

