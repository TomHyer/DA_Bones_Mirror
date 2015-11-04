
// Monte Carlo driver for pricing

#pragma once

#include "Valuation.h"
#include "ValueRequest.h"
#include "Payout.h"
#include "Step.h"

class SDE_;

namespace MonteCarlo
{
	class Task_ : public ReEvaluator_
	{
		scoped_ptr<ValueRequest_> request_;
		scoped_ptr<const Payout_> payout_;
		scoped_ptr<StepAccumulator_> cumulant_;
		Vector_<Handle_<ModelStepper_> > steps_;
		scoped_ptr<PathsRecord_> paths_;

	public:
		Task_(const SDE_& model,
			 ValueRequest_* request,
			 const Payout_* payout);
	};
}