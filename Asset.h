
// a model's version of a financial quantity
	// created by models, updated using model state, viewed by trades

#pragma once

#include "AssetValue.h"

class Asset_ : noncopyable
{
public:
	virtual UpdateToken_ Update
		(const DateTime_& event_time,
		 const Vector_<>& state) = 0;
};

