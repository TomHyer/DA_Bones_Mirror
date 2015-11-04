
#include "Platform.h"
#include "IndexIr.h"
#include "Strict.h"

#include "Conventions.h"
#include "DateIncrement.h"
#include "Exceptions.h"

namespace
{
	Date_ DateFromCell(const Cell_& c, const Date_& fix)
	{
		switch (c.type_)
		{
		case Cell_::Type_::EMPTY:
			return fix;
		case Cell_::Type_::NUMBER:
			REQUIRE(static_cast<int>(c.d_) == c.d_ && c.d_ >= 0, "Start delay days must be a nonnegative integer");
			return fix.AddDays(static_cast<int>(c.d_));
		case Cell_::Type_::DATE:
			return c.dt_.Date();
		case Cell_::Type_::STRING:
			return Date::ParseIncrement(c.s_)->FwdFrom(fix);
		default:
			THROW("Invalid type for date offset");
		}
	}

	String_ MatPostfix(const Cell_& start)
	{
		// incomplete implementation
		switch (start.type_)
		{
		case Cell_::Type_::STRING:
			return start.s_;
		case Cell_::Type_::EMPTY:
			THROW("Maturity may not be empty")
		default:
			THROW("Unsupported start/mat type in index");
		}
	}
	String_ StartPostfix(const Cell_& start)
	{
		switch (start.type_)
		{
		case Cell_::Type_::EMPTY:
			return String_();
		default:
			return "," + MatPostfix(start);
		}
	}
}

Date_ Index::IRForward_::StartDate(const DateTime_& fixing_time) const
{
	Date_ temp = DateFromCell(start_, fixing_time.Date());
	if (Cell::IsDate(start_))
		return temp;	// start date explicitly supplied
	return Libor::StartFromFix(ccy_, temp);
}

String_ Index::Libor_::Name() const
{
	return "IR:" + String_(ccy_.String()) + "," + String_(tenor_.String()) + StartPostfix(start_);
}

String_ Index::Swap_::Name() const
{
	return "IR:" + String_(ccy_.String()) + "," + tenor_ + StartPostfix(start_);	// note ",5Y" is a swap, ",Libor3M" is a Libor -- numeric first digit indicates a swap
}

String_ Index::DF_::Name() const
{
	return "IR[DF]:" + String_(ccy_.String()) + StartPostfix(start_) + ":" + MatPostfix(maturity_);
}

Date_ Index::DF_::StartDate(const DateTime_& fixing_time) const
{
	return DateFromCell(start_, fixing_time.Date());
}

Date_ Index::DF_::Maturity(const DateTime_& fixing_time) const
{
	return DateFromCell(maturity_, fixing_time.Date());
}

