
// standard conventions (usually per-currency) for date math
	// relies on facts stored in CurrencyData

#pragma once

class Date_;
class DateTime_;
class Ccy_;

namespace Libor
{
	Date_ StartFromFix(const Ccy_& ccy, const Date_& fix_date);
	DateTime_ FixFromStart(const Ccy_& ccy, const Date_& start_date);
}