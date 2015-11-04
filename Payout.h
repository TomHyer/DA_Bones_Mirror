
// payout function, created by a trade, for numerical pricing

#pragma once

#include "Payment.h"
#include "AssetValue.h"
namespace BackwardInduction { struct Action_; }
struct ObservedDefault_;

class Payout_ : noncopyable
{
public:
	typedef Handle_<Payment::Tag_> dst_t;
	typedef Handle_<Payment::Amount::Tag_> amount_t;
	virtual ~Payout_();

	virtual Vector_<DateTime_> EventTimes() const = 0;

	class State_ : noncopyable
	{
	public:
		virtual ~State_();
		virtual State_* Clone() const = 0;
		virtual State_& operator=(const State_& rhs) = 0;
	};
	// default implementation is for stateless (non-path-dependent) trades
	virtual State_* NewState() const { return nullptr; }
	virtual void StartPath(State_* state) const {}

	virtual void DoNode
		(const UpdateToken_& values,
		 State_* state,
		 NodeValues_& pay_dst)
	const = 0;

	virtual void DoDefault
		(const ObservedDefault_& event,
		 State_* state,
		 const NodeValuesDefault_& pay_dst)
	const
	{	}	// default implementation is no-op

	virtual Vector_<BackwardInduction::Action_>	BackwardSteps() const = 0;

	// components of value for each trade name
	typedef std::map<String_, Vector_<pair<String_, double>>> weights_t;
	virtual weights_t StreamWeights() const = 0;
};

namespace Payout
{
	Payout_::weights_t IdentityWeight(const String_& name);
}