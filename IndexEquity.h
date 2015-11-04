
// "index" (model's time series) of an equity price

#pragma once

#include "Index.h"
#include "Cell.h"

namespace Index
{
	class Equity_ : public Index_	// equity price (spot or forward)
	{
		Cell_ delivery_;   // empty, date, or increment
		String_ Name() const;
	public:
		const String_ eqName_;
		Date_ Delivery(const DateTime_& fixing_time) const;

		// can't supply both date and increment!
		Equity_(const String_& eq_name,
			const Date_* delivery_date = nullptr,
			const String_* delay_increment = nullptr);
	};
}

