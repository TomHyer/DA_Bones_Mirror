
// indices of implied vol of an underlying index

#pragma once

#include "Index.h"

class IndexIV_ : public Index_
{
	Handle_<Index_> underlying_;   // no trade/fixing ID
	Cell_ expiry;      // datetime or maturity tenor
	boost::optional<double> callDelta_;   // otherwise ATM
	VolType_ volType_;
public:
	String_ Name() const;
	// help models find what is expected
	const Index_& Underlying() const { return *underlying_; }
	boost::optional<double> CallDelta(bool invert = false) const;
	DateTime_ Expiry(const DateTime_& fixing_time) const;
	const VolType_& VolType() const { return volType_; }

	IndexIv_
		(const Handle_<Index_>& underlying,
		 const VolType_& vol_type,
		 const Date_& expiry_date,
		 const double* call_delta = nullptr);
	IndexIv_
		(const Handle_<Index_>& underlying,
		 const VolType_& vol_type,
		 const String_& expiry_tenor,
		 const double* call_delta = nullptr);
};
