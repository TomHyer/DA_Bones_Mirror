
// model's simulation of asset-value observations required by a payout

#pragma once

#include "Vectors.h"
#include "IndexPath.h"
#include "AssetValue.h"

struct PathFixings_ : noncopyable
{
	Vector_<> vals_;
   Vector_<std:: shared_ptr<IndexPath_> > paths_;
	Vector_<Handle_<IndexPath_> > pathRO_;
	int valMask_, dateMask_;

	UpdateToken_ Token(const DateTime_& evt_t) const
	{
		return UpdateToken_
				(vals_.begin(), pathRO_.begin(), valMask_, dateMask_, evt_t);
	}
};

