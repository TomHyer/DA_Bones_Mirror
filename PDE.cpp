
#include "Platform.h"
#include "PDE.h"
#include "Strict.h"

#include "Algorithms.h"

PDE::CoordinateMap_::~CoordinateMap_()
{	}

PDE::Rollback_::~Rollback_()
{	}

PDE::Coeff_::~Coeff_()
{	}

namespace
{
	struct IdentityMap_ : PDE::CoordinateMap_
	{
		double operator()(double y, double* xp = nullptr, double* xpp = nullptr) const override
		{
			ASSIGN(xp, 1.0);
			ASSIGN(xpp, 0.0);
			return y;
		}
		double Y(double x) const override { return x; }
	};

	struct SinhMap_ : PDE::CoordinateMap_
	{
		//x = \lambda sinh(y / \lambda)
		double lambda_;
		SinhMap_(double lambda) : lambda_(lambda) {}
		double operator()(double y, double* xp = nullptr, double* xpp = nullptr) const override
		{
			ASSIGN(xp, cosh(y / lambda_));
			ASSIGN(xpp, sinh(y / lambda_) / lambda_);
			return lambda_ * sinh(y / lambda_);
		}
		double Y(double x) const override {	return lambda_ * asinh(x / lambda_); }
	};
}	// leave local

PDE::CoordinateMap_* PDE::NewSinhMap
	(double x_width,
	 double dxdy_range)
{
	assert(IsPositive(x_width) && dxdy_range >= 1.0);
	double sinhMaxY = sqrt(Square(dxdy_range) - 1.0);
	return IsZero(Square(sinhMaxY))
			? (CoordinateMap_*) new IdentityMap_
			: new SinhMap_(x_width / sinhMaxY);
}
