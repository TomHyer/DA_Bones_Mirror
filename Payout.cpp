

#include "Platform.h"
#include "Payout.h"
#include "Strict.h"

#include "BackwardInduction.h"

Payout_::~Payout_()
{	}

Vector_<BackwardInduction::Action_>	Payout_::BackwardSteps() const
{
	return Vector_<BackwardInduction::Action_>();
}

Payout_::weights_t Payout::IdentityWeight(const String_& name)
{
	Payout_::weights_t r;
	r[name] = Vector::V1(make_pair(name, 1.0));
	return r;
}

