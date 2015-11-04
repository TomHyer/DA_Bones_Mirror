
#pragma once

#include <map>
#include "Archive.h"

class Interp1_ : public Storable_
{
public:
	Interp1_(const String_& name);
	virtual double operator()(double x) const = 0;
	virtual bool IsInBounds(double x) const { return true; }
};

/*IF--------------------------------------------------------------------------
storable Interp1Linear
	Linear interpolator on known values in one dimension
version 1
&members
name is ?string
x is number[]
f is number[]
-IF-------------------------------------------------------------------------*/

class Interp1Linear_ : public Interp1_
{
	Vector_<> x_, f_;
public:
	Interp1Linear_(const String_& name, const Vector_<>& x, const Vector_<>& f);
	Interp1Linear_(const String_& name, const std::map<double, double>& f);
	void Write(Archive::Store_& dst) const override;
	double operator()(double x) const override;
};

class Interp2_ : public Storable_
{
public:
	Interp2_(const String_& name);
	virtual double operator()(double x1, double x2) const = 0;
	virtual bool IsInBounds(double x1, double x2) const = 0;
};

namespace Interp1
{
	template<class T_> struct Of_
	{
		Handle_<Interp1_> imp_;
		Of_(const Handle_<Interp1_>& imp) : imp_(imp) {}
		double operator()(const T_& x) const { return (*imp_)(NumericValueOf(x)); }
	};
}
namespace Interp2
{
	template<class T1_, class T2_ = double> struct Of_
	{
		Handle_<Interp2_> imp_;
		Of_(const Handle_<Interp2_>& imp) : imp_(imp) {}
		double operator()(const T1_& x1, const T2_& x2) const { return (*imp_)(NumericValueOf(x1), NumericValueOf(x2)); }
	};
}

