
#include "Platform.h"
#include "IndexEquity.h"
#include "Strict.h"

String_ Index::Equity_::Name() const
{
	String_ ret = "EQ[" + eqName_ + "]";
	if (Cell::IsString(delivery_))
		ret += ">" + Cell::OwnString(delivery_);
	else if (Cell::IsDate(delivery_))
		ret += "@" + Date::ToString(Cell::ToDate(delivery_));
	return ret;
}

