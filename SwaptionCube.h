
// swaption vol (or vol-like parameter) as a function of expiry, tenor, strike

#pragma once

#include "Model.h"

class SwaptionCube_ : public Model_
{
	Handle_<SDE_> ForTrade
		(_ENV, const Underlying_& underlying)
	const override;
};

