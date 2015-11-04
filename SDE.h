
// specialization of a model to a single pricing task
	// with known event times, underlyings, and value ccy

#pragma once

class ValueRequest_;
class Asset_;
class StepAccumulator_;
class ModelStepper_;
class DateTime_;

class SDE_ : noncopyable
{
public:
	virtual ~SDE_();
	virtual ValueRequest_* NewRequest() const = 0;
	virtual Asset_* NewAsset(ValueRequest_& req) const = 0;

	virtual StepAccumulator_* NewAccumulator() const = 0;
	virtual ModelStepper_* NewStepper
		(const DateTime_& from,
		const DateTime_& to,
		StepAccumulator_* cumulative,
		ModelStepper_* exemplar)
	const = 0;
};
