
// passing of values in response to trade's requests

#pragma once

#include "Optionals.h"
#include "Vectors.h"
#include "IndexPath.h"

namespace Valuation
{
	typedef size_t address_t;
	boost::optional<double> KnownValue(address_t loc);
	boost::optional<address_t> FixedLoc(double value);

	struct IndexAddress_
	{
		int val_;
		IndexAddress_(int val) : val_(val) {}
	};
}

class UpdateToken_
{
	typedef Vector_<Handle_<IndexPath_> > indices_t;
	Vector_<>::const_iterator begin_;
	indices_t::const_iterator indexBegin_;
	const int valMask_, dateMask_;
public:
	const DateTime_ eventTime_;
	UpdateToken_
		(Vector_<>::const_iterator begin,
		 indices_t::const_iterator index_begin,
		 int val_mask, 
		 int date_mask,
		 const DateTime_& event_time);

	inline const double& operator[](const Valuation::address_t& loc) const
	{
		// loc&dateMask_ can be checked with assert
		return *(begin_ + (loc & valMask_));
	}
	const IndexPath_& Index(const Valuation::IndexAddress_&) const;
};

