
// backward induction actions specified in a derivative trade

#pragma once

#include <boost/variant.hpp>
#include "Payment.h"

namespace BackwardInduction
{
	using Payment::Tag_;   // for fees
	typedef Payment::Amount::Tag_ amount_t;

	struct StreamSegment_
	{
		String_ stream_;
		Date_ deliveryDate_, terminationDate_;
		StreamSegment_
			(const String_& s, 
			 const Date_& delivery,
			 const Date_& termination = Date::Maximum());
		StreamSegment_();   // support vectors
	};

	// Different concrete actions
	struct Exercise_
	{
		int sign_;
		// where to find the values
		Vector_<Handle_<Tag_> > fees_;
		Vector_<StreamSegment_> underlyings_;
		Vector_<Handle_<amount_t> > observables_;
		Vector_<Handle_<amount_t> > slaves_;   // write exProb
	};

	struct Barrier_
	{
		Vector_<Handle_<Tag_> > payOnHit_;
		Vector_<StreamSegment_> knockIn_;
		Handle_<amount_t> hitProb_;
	};

	struct Include_
	{
		Vector_<StreamSegment_> src_;
	};

	struct Action_
	{
		String_ stream_;
		DateTime_ eventTime_;   // not strictly necessary
		Date_ deliveryDate_;
		boost::variant<Exercise_, Barrier_, Include_, Empty_> details_;	// I loathe Boost variant -- it is heavy and awkward -- but we need the type checking that every case is covered
		Action_
			(const String_& stream,
			 const DateTime_& event, 
			 const Date_& delivery);
		Action_();   // support Vector_<Action_>
	};
}   // leave namespace BackwardInduction

