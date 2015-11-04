
// compact date -- stores integer dates from 1/1/1970 to 6/5/2149

#pragma once

#include <cstdint>
#include "Strings.h"	// forward declaration only works for non-std classes

class Date_;
int operator-(const Date_& lhs, const Date_& rhs);

namespace Date
{
	short Year(const Date_& dt);
	short Month(const Date_& dt);
	short Day(const Date_& dt);
	short DayOfWeek(const Date_& dt);	// 0 == Sunday
	short DaysInMonth(int year, int month);
	inline bool IsWeekend(const Date_& dt) { return DayOfWeek(dt) % 6 == 0; }

	Date_ FromExcel(int serial);	// converts from serial date with Excel offset (ignores Excel bug for pre-1902 dates)
	int ToExcel(const Date_& dt);	// extracts serial date with Excel offset

	String_ ToString(const Date_& date);	// always yyyy-mm-dd
	// FromString is higher-level functionality, moved to DateUtils
	Date_ Minimum();
	Date_ Maximum();

	Date_ EndOfMonth(const Date_& date);
	Date_ AddMonths(const Date_& date, int n_months, bool preserve_eom = false);
}

class Date_
{
	uint16_t serial_;	// days from start of unix era
	friend Date_ Date::FromExcel(int);
	friend int Date::ToExcel(const Date_&);
	friend bool operator==(const Date_& lhs, const Date_& rhs);
	friend bool operator<(const Date_& lhs, const Date_& rhs);
public:
	Date_() : serial_(0) {}	// invalid value
	Date_(int yyyy, int mm, int dd);
	Date_(const Date_& src) : serial_(src.serial_) {}

	bool IsValid() const { return serial_ > 0; }
	Date_& operator=(const Date_& rhs) { serial_ = rhs.serial_; return *this; }
	inline Date_ AddDays(int days) const { Date_ retval(*this); retval.serial_ += static_cast<short>(days); return retval; }	// note this creates a new date; it is operator+, not operator+=
	inline Date_& operator++() { ++serial_; return *this; }
	inline Date_& operator--() { --serial_; return *this; }
};

inline bool operator==(const Date_& lhs, const Date_& rhs) { return lhs.serial_ == rhs.serial_; }
inline bool operator!=(const Date_& lhs, const Date_& rhs) { return !(lhs == rhs); }
inline bool operator<(const Date_& lhs, const Date_& rhs) { return lhs.serial_ < rhs.serial_; }
inline bool operator>(const Date_& lhs, const Date_& rhs) { return rhs < lhs; }
inline bool operator<=(const Date_& lhs, const Date_& rhs) { return !(rhs < lhs); }
inline bool operator>=(const Date_& lhs, const Date_& rhs) { return !(lhs < rhs); }
int operator-(const Date_& lhs, const Date_& rhs);

// support interpolation based on date
inline double NumericValueOf(const Date_& src) { return Date::ToExcel(src); }
