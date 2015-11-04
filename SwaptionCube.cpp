
#include "Platform.h"
#include "SwaptionCube.h"
#include "Strict.h"

#include "Exceptions.h"
#include "Archive.h"
#include "Audit.h"
#include "Underlying.h"
#include "ValuationMethod.h"
#include "Swaption.h"
#include "BermudanSwaption.h"
#include "Semianalytic.h"
#include "VHWCalibrate.h"

namespace
{
/*IF--------------------------------------------------------------------------
storable IntAsStorable
	Storable wrapper for an integer
&members
i is integer
-IF-------------------------------------------------------------------------*/
#include "MG_IntAsStorable_Write.inc"

	struct IntAsStorable_ : Storable_
	{
		int i_;
      IntAsStorable_(int i) : Storable_("IntAsStorable", String_()), i_(i) {}

		void Write(Archive::Store_& dst) const override
		{
         IntAsStorable::XWrite(dst, i_);
		}
	};

#include "MG_IntAsStorable_Read.inc"

	int MVEIndex(_ENV, const void* pp, const Vector_<>& euro_vals)
	{
      const String_ key = "MVEIndexForBerm" + String::Uniquifier(pp);
		Handle_<IntAsStorable_> store;
		Environment::Recall(_env, key, &store);
		if (store.Empty())
		{
			const int i = static_cast<int>(MaxElement(euro_vals) - euro_vals.begin());
			store.reset(new IntAsStorable_(i));
			Environment::Audit(_env, key, store);
		}
		return store->i_;
	}
}	// leave local

Handle_<SDE_> SwaptionCube_::ForTrade
	(_ENV, const Underlying_& underlying)
const
{
	auto berm = handle_cast<HasEuropeanComponents_>(underlying.parent_);
	REQUIRE(!berm.Empty(), "Can't price non-Bermudan option numerically");
	auto euros = berm->EuropeanComponents();
	assert(!euros.empty());
	auto evaluate = [&](const Handle_<Swaption_>& ec) {return Semianalytic::Value(_env, *ec, *this).front().second; };	// just the number
	auto euroVals = Apply(evaluate, euros);
	const int iMVE = MVEIndex(_env, berm.get(), euroVals);
	const Swaption_& mve = *euros[iMVE];
	scoped_ptr<Model_> vhw
			(VHW::MatchSwaptionHoLee
					(String_(), YieldCurve(mve.valueCcy_), mve, euroVals[iMVE], VolStart()));
	return vhw->ForTrade(_env, underlying);	// just returns itself, a VHW is also an SDE
}

