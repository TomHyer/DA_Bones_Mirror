
// PDE schemes, operating through time on "slabs" which give the space-dependence of the data

#pragma once

#include <map>
#include <bitset>
#include "DateTime.h"
#include "Archive.h"

class Cube_;

namespace PDE
{
	class CoordinateMap_ : noncopyable
	{
	public:
		virtual ~CoordinateMap_();
		virtual double operator()
			(double y, double* dx_dy = nullptr, double* d2x_dy2 = nullptr)
		const = 0;
		virtual double Y(double x) const = 0;
	};

	// some particular maps
	CoordinateMap_* NewSinhMap
		(double x_width,
		 double dxdy_range);
	inline CoordinateMap_* NewIdentityMap() { return NewSinhMap(1.0, 1.0); }

	struct CoordinateVector_
	{
		double yLow_, yHigh_;
		int n;   // n >= 2 if low != high
		Handle_<CoordinateMap_> yToX_;
		std::map<DateTime_, double> rescalings_;
	};

	// coefficients
	static const size_t MAX_DIMENSIONS = 3;
	class Coeff_
	{
	public:
		virtual ~Coeff_();
		typedef std::bitset<MAX_DIMENSIONS> x_dep_t;
	};

	class MatrixCoeff_ : public Coeff_
	{
	public:
		virtual void Value
			(const Vector_<>& x,
			 SquareMatrix_<>* value)
		const = 0;

		virtual Matrix_<x_dep_t> XDependence() const = 0;
	};
	MatrixCoeff_* NewConstCoeff(const Matrix_<>& val);

	class VectorCoeff_ : public Coeff_
	{
	public:
		virtual void Value
			(const Vector_<>& x,
			 Vector_<>* value)
		const = 0;

		virtual Vector_<x_dep_t> XDependence() const = 0;
	};
	VectorCoeff_* NewConstCoeff(const Vector_<>& val);

	class ScalarCoeff_ : public Coeff_
	{
	public:
		virtual void Value
			(const Vector_<>& x,
			 double* value)
		const = 0;

		virtual x_dep_t XDependence() const = 0;
	};
	ScalarCoeff_* NewConstCoeff(double val);

	class Rollback_
	{
	public:
		virtual ~Rollback_();

      virtual void operator()
			(double dt,      // positive
			 const Vector_<CoordinateVector_>& x_points,
			 const Vector_<std::shared_ptr<Cube_> >& old_vals,
			 const ScalarCoeff_& discounting,
			 const VectorCoeff_& advection,
			 const MatrixCoeff_& diffusion,
			 Vector_<std::shared_ptr<Cube_> >* new_vals)
		const = 0;
	};
}

