
#include "Platform.h"
#include "Interp.h"
#include "Strict.h"

#include "Functionals.h"
#include "Exceptions.h"

Interp1_::Interp1_(const String_& name) : Storable_("Interp1", name) {}

Interp1Linear_::Interp1Linear_(const String_& name, const Vector_<>& x, const Vector_<>& f) : Interp1_(name), x_(x), f_(f)
{
	assert(x.size() == f.size());
	assert(IsMonotonic(x, std::less_equal<double>()));
	if (!IsMonotonic(x))
	{
		x_.clear();
		f_.clear();
		for (int ii = 0; ii < x.size(); ++ii)
		{
			if (x_.empty() || x[ii] > x_.back())
			{
				x_.push_back(x[ii]);
				f_.push_back(f[ii]);
			}
		}
	}
}

Interp1Linear_::Interp1Linear_(const String_& name, const std::map<double, double>& f)
:
Interp1_(name),
x_(Keys(f)),
f_(MapValues(f))
{	}

double Interp1Linear_::operator()(double x) const
{
	auto pge = LowerBound(x_, x);
	if (pge == x_.end())
		return f_.back();
	else if (pge == x_.begin() || IsZero(x - *pge))
		return f_[pge - x_.begin()];
	else
	{
		auto plt = Previous(pge);
		const double gFrac = (x - *plt) / (*pge - *plt);
		auto flt = f_.begin() + (plt - x_.begin());
		return *flt + gFrac * (*Next(flt) - *flt);
	}
}

namespace
{
#include "MG_Interp1Linear_v1_Write.inc"
#include "MG_Interp1Linear_v1_Read.inc"
}	// leave local

void Interp1Linear_::Write(Archive::Store_& dst) const
{
   Interp1Linear_v1::XWrite(dst, name_, x_, f_);
}

