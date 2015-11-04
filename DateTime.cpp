
#include "Platform.h"
#include "DateTime.h"
#include <tuple>
#include "Strict.h"

#include "Algorithms.h"

namespace
{
	void FracToHMS(double frac, int* h, int* m, int* s)
	{
		ASSIGN(h, static_cast<int>(24 * frac));
		ASSIGN(m, static_cast<int>(1440 * frac) % 60);
		ASSIGN(s, static_cast<int>(86400 * frac) % 60);
	}
}

DateTime_::DateTime_(const Date_& date, double frac) 
: 
date_(date), 
frac_(static_cast<uint16_t>(Min(65535.0, 65536 * frac + 0.5)))
{	
	assert(frac >= 0.0 && frac < 1.0);
}

DateTime_::DateTime_(const Date_& dt, int h, int m, int s)
:
date_(dt)
{
	const int secs = 60 * (60 * h + m) + s;
	assert(secs >= 0 && secs < 86400);
	const double frac = (secs + 0.65) / 86400.0;	// half a second at midnight will be rounded to zero in the next step
	frac_ = static_cast<uint16_t>(65536 * frac);
}

DateTime_::DateTime_(long long msec)
{
	auto whole = msec / 86400000;
	auto frac = ((msec + 500 - 86400000 * whole) * 64) / 84375;	// 86400000/2^16 = 84375/64
	assert(frac <= 0xFFFF);
	frac_ = static_cast<uint16_t>(frac);
	date_ = Date::Minimum().AddDays(static_cast<int>(whole));
}

String_ DateTime::TimeString(const DateTime_& dt)
{
	int h, m, s;
	FracToHMS(dt.Frac(), &h, &m, &s);
	String_ retval("00:00:00");
	sprintf(&retval[0], "%2d:%2d:%2d", h, m, s);
	for (int ii = 0; ii < 8; ii += 3)
		if (retval[ii] == ' ')
			retval[ii] = '0';
	return retval;
}

bool operator<(const DateTime_& lhs, const DateTime_& rhs)
{
   return lhs.Date() < rhs.Date()
      || (lhs.Date() == rhs.Date() && lhs.Frac() < rhs.Frac());
}

DateTime_ DateTime::Minimum()
{
	return DateTime_(Date::Minimum(), 0);
}

double operator-(const DateTime_& lhs, const DateTime_& rhs)
{
	return (lhs.Date() - rhs.Date()) + (lhs.Frac() - rhs.Frac());
}

long long DateTime::MSec(const DateTime_& dt)
{
	long long days = dt.Date() - Date::Minimum();
	return 86400000 * days + static_cast<long long>(86400000 * dt.Frac());
}
