
#include "Platform.h"
#include "Payment.h"
#include "Strict.h"

Payment::Conditions_::Conditions_()
:
exerciseCondition_(Exercise_::UNCONDITIONAL),
creditCondition_(Credit_::RISKLESS)
{	}


Payment::Info_::Info_
	(const String_& des,
	 const DateTime_& known,
	 const Conditions_& cond,
	 const AccrualPeriod_* accrual)
	 :
description_(des),
knownTime_(known),
conditions_(cond)
{
	if (accrual)
		period_ = *accrual;
}

Payment_::Payment_()
{	}

Payment_::Payment_
	(const DateTime_& et,
	 const Ccy_& ccy,
	 const Date_& dt,
	 const String_& s,
	 const Payment::Info_& tag,
	 const Date_& cd)
:
eventTime_(et),
ccy_(ccy),
date_(dt),
stream_(s),
tag_(tag),
commitDate_(cd)
{	}

// special tag (with shared address so we can test equality) for meaningless payments
const Handle_<Payment::Tag_>& Payment::Null()
{
   RETURN_STATIC(const Handle_<Tag_>);
}