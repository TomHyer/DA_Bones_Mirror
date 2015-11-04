
// higher-level date functions (e.g., those that might throw)

class String_;
class Date_;

namespace Date
{
	bool IsDateString(const String_& src);	// predicts whether FromString will work -- examines format only
	Date_ FromString(const String_& src);	// tries our best to recognize the string -- rejects both mm/dd/yyyy and dd/mm/yyyy due to ambiguity
	int MonthFromFutureCode(char code);
}

