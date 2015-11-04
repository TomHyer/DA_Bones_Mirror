
// base class for decorators implementing Payout_

#pragma once

#include "Payout.h"

// base decorator behaves exactly like its base_ member
struct PayoutDecorated_ : Payout_
{
	Handle_<Payout_> base_;
public:
	PayoutDecorated_(const Handle_<Payout_>& base) : base_(base) {}

	Vector_<DateTime_> EventTimes() const override { return base_->EventTimes(); }

	State_* NewState() const override { return base_->NewState(); }
	void StartPath(State_* state) const override { base_->StartPath(state); }

	void DoNode(const UpdateToken_& values, State_* state, NodeValues_& pay_dst) const override { base_->DoNode(values, state, pay_dst); }
	void DoDefault(const ObservedDefault_& event, State_* state, const NodeValuesDefault_& pay_dst) const override { base_->DoDefault(event, state, pay_dst); }

	Vector_<BackwardInduction::Action_>	BackwardSteps() const override { return base_->BackwardSteps(); }

	weights_t StreamWeights() const override { return base_->StreamWeights(); }
};

