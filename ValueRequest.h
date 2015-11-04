
// interface called by the trade to obtain asset values it needs

#pragma once

#include "Payment.h"
#include "AssetValue.h"
class Index_;

class ValueRequest_
{
public:
	virtual ~ValueRequest_();

	virtual Handle_<Payment::Tag_> PayDst(const Payment_& flow) = 0;

	virtual Handle_<Payment::Default::Tag_> DefaultDst(const String_& stream) = 0;

	virtual Valuation::address_t Fixing
		(const DateTime_& event_time,
		const Index_& index) = 0;

	virtual Valuation::IndexAddress_ IndexPath
		(const DateTime_& last_event_time,
		const Index_& index) = 0;

	typedef Valuation::address_t address_t;
	typedef Valuation::IndexAddress_ IndexAddress_;
};

