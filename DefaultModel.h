
// default event plus extra modeling information
	// trades should not use this header -- it is for the model to communicate to the numerical pricer

#pragma once

#include "Default.h"

class DefaultEvent_ : noncopyable
{
protected:
	DefaultEvent_
		(const Date_& date, 
		 const CreditId_& which,
		 double recovery, 
		 double df_from_event);
public:
	virtual ~DefaultEvent_();

	const ObservedDefault_ observed_;   // seen by trade
	const double dfFromPreviousEvent_;   // seen by MC
};
