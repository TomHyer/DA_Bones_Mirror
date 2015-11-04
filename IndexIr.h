
// interest-rate indices (Libor, par rates, discounts)

#pragma once

#include "Functionals.h"
#include "Optionals.h"
#include "Cell.h"
#include "Currency.h"
#include "CollateralType.h"
#include "CouponRate.h"
#include "Index.h"

namespace Index
{
	// Cell_ is used widely to indicate start/end dates; it can be 
		// -- a fixed Date_
		// -- a double containing an integer number of calendar days
		// -- a tenor/increment string
	// internally we switch on cell type to compute observable quantities

	class IRForward_ : public Index_
	{
	public:
		const Ccy_ ccy_;
		const Cell_ start_;	// fixed start date, or number of days offset, or tenor offset -- leave empty for the usual spot index
		IRForward_(const Ccy_& ccy, const Cell_& start = Cell_()) : ccy_(ccy), start_(start) {}

		Date_ StartDate(const DateTime_& fixing_time) const;
	};

	class Libor_ : public IRForward_
	{
	public:
		const TradedRate_ tenor_;
		Libor_(const Ccy_& ccy, const TradedRate_& tenor, const Cell_& start = Cell_()) : IRForward_(ccy, start), tenor_(tenor) {}

		String_ Name() const override;
	};
	class Swap_ : public IRForward_
	{
	public:
		const String_ tenor_;	// should be parseable as a Date::Increment
		Swap_(const Ccy_& ccy, const String_& tenor, const Cell_& start = Cell_()) : IRForward_(ccy, start), tenor_(tenor) {}

		String_ Name() const override;
	};

	class DF_ : public Index_
	{
	public:
		const Ccy_ ccy_;
		const Cell_ maturity_;	// fixed end date, or days/tenor offset -- offset is from fixing, NOT from start date (i.e., "2Y" start and "3Y" maturity does not lead to 5y final maturity)
		const CollateralType_ collateral_;
		Cell_ start_;	// fixed start date, or number of days offset, or tenor offset -- leave empty for the usual spot index

		DF_(const Ccy_& ccy, const Cell_& maturity, const Cell_* start = nullptr, const CollateralType_& coll = CollateralType_()) : ccy_(ccy), maturity_(maturity), start_(Dereference(start, Cell_())) {}

		String_ Name() const override;
		Date_ Maturity(const DateTime_& event_time) const;
		Date_ StartDate(const DateTime_& event_time) const;
	};
}

