
// FX (spot and forward) indices

#pragma once

#include "Index.h"
#include "Currency.h"

namespace Index
{
	class Fx_ : public Index_
	{
      Ccy_ dom_, fgn_;
      String_ XName(bool invert) const;
	public:
      String_ Name() const override { return XName(false); }
		double Fixing(_ENV, const DateTime_& fixing_time) const override;
	};
}
