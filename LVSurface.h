
// abstract interface of a local volatility surface

#pragma once

#include "Storable.h"

class DateTime_;
class StepAccumulator_;

class LVSurface_ : public Storable_
{
public:
	virtual DateTime_ VolStartTime() const = 0;
	virtual double LocalVol(const DateTime_& t, double s)
		const = 0;
	virtual double IntervalVol(const DateTime_& t_minus,
		const DateTime_& t_plus, double s) const;

	virtual StepAccumulator_* NewAccumulator() const = 0;
	virtual pair<double, double> UpdateEnvelope
		(StepAccumulator_* accumulator,
		 const DateTime_& t,
		 double num_sigma)
	const = 0;
	// ...
};
