
// model contains a parametrization of the dynamics of some part of the market
	// produces SDE for pricing on demand

#pragma once

#include "Environment.h"
#include "Storable.h"

class DateTime_;
class SDE_;
struct Underlying_;
class YieldCurve_;
class Ccy_;
class String_;
class Slide_;

class Model_ : public Storable_
{
	// single slide
	virtual Model_* Mutant_Model
		(const String_* new_name = nullptr,
		 const Slide_* slide = nullptr)
	const = 0;
public:
	Model_(const String_& name) : Storable_("Model", name) {}

	virtual Handle_<SDE_> ForTrade
		(_ENV, const Underlying_& trade)
	const = 0;

	virtual Handle_<YieldCurve_> YieldCurve
		(const Ccy_& ccy)
	const = 0;

	virtual DateTime_ VolStart() const = 0;

	Model_* Mutant_Model
		(const String_& new_name,
		 const Vector_<Handle_<Slide_> >& slides)
	const;
};
