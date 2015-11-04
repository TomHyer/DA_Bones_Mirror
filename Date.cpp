
#include "Platform.h"
#include "Date.h"
#include "Strict.h"

#include "Algorithms.h"
#include "Vectors.h"

namespace
{
	static const int EXCEL_OFFSET = 25568;

	void ExcelDateToYMD(long serial, short* yy, short* mm, short* dd)
	{
		// based on Fenton's pseudocode of Meeus's method
		serial += 2415019;
		const int alpha = static_cast<int>((serial - 1867216.25) / 36524.25);
		const int A = serial + 1 + alpha - (alpha / 4);
		const int B = A + 1524;
		const int C = static_cast<int>((B - 122.1) / 365.25);
		const int D = static_cast<int>(365.25 * C);
		const int E = static_cast<int>((B - D) / 30.6001);
		ASSIGN(dd, static_cast<short>(B - D - static_cast<int>(30.6001 * E)));
		const int m = E - (E > 13 ? 13 : 1);
		ASSIGN(mm, static_cast<short>(m));
		ASSIGN(yy, static_cast<short>(C - (m > 2.5 ? 4716 : 4715)));
	}

	int ExcelDateFromYMD(int yy, int mm, int dd)
	{
		// based on Vogelpoel's port of now-lost pseudocode
		// ignore Excel bug around 29 Feb 1900 -- dates before then will be wrong

		return int((1461 * (yy + int((mm - 14) / 12))) / 4) +
			int((367 * (mm - 2 - 12 * ((mm - 14) / 12))) / 12) -
			int((3 * (int((yy + 4900 + int((mm - 14) / 12)) / 100))) / 4) +
			dd - 693894l;
	}

	uint16_t SerialFromYMD(int yy, int mm, int dd)
	{
		const int xl = ExcelDateFromYMD(yy, mm, dd);
		uint16_t retval = static_cast<uint16_t>(xl - EXCEL_OFFSET);
		return static_cast<int>(retval)+EXCEL_OFFSET == xl
				? retval
				: 0;		// equality failure means overflow of uint16
	}
}	// leave local

Date_::Date_(int yy, int mm, int dd)
:
serial_(SerialFromYMD(yy, mm, dd))
{	}

Date_ Date::FromExcel(int serial)
{
	Date_ retval;
	if (serial > EXCEL_OFFSET && serial < EXCEL_OFFSET + (1 << 16))
		retval.serial_ = static_cast<uint16_t>(serial - EXCEL_OFFSET);	
	// otherwise leave it invalid
	return retval;
}
int Date::ToExcel(const Date_& dt)
{
	return dt.serial_ + EXCEL_OFFSET;
}

short Date::Year(const Date_& dt)
{
	short yy;
	ExcelDateToYMD(ToExcel(dt), &yy, nullptr, nullptr);
	return yy;
}
short Date::Month(const Date_& dt)
{
	short mm;
	ExcelDateToYMD(ToExcel(dt), nullptr, &mm, nullptr);
	return mm;
}
short Date::Day(const Date_& dt)
{
	short dd;
	ExcelDateToYMD(ToExcel(dt), nullptr, nullptr, &dd);
	return dd;
}

String_ Date::ToString(const Date_& dt)
{
	short yy, mm, dd;
	ExcelDateToYMD(ToExcel(dt), &yy, &mm, &dd);
	String_ retval("0000-00-00");
	sprintf(&retval[0], "%4d-%2d-%2d", int(yy), int(mm), int(dd));
	if (retval[5] == ' ')
		retval[5] = '0';
	if (retval[8] == ' ')
		retval[8] = '0';
	return retval;
}

short Date::DayOfWeek(const Date_& dt)
{
	static const int KNOWN_SUNDAY = 974;
	return (ToExcel(dt) - KNOWN_SUNDAY) % 7;
}

short Date::DaysInMonth(int year, int month)
{
	return month == 12
		? 31
		: static_cast<short>(Date_(year, month + 1, 1) - Date_(year, month, 1));
}

Date_ Date::EndOfMonth(const Date_& dt)
{
	const int yy = Year(dt), mm = Month(dt);
	return mm == 12
			? Date_(yy + 1, 1, 1).AddDays(-1)
			: Date_(yy, mm + 1, 1).AddDays(-1);
}

Date_ Date::AddMonths(const Date_& dt, int n_months, bool preserve_eom)
{
	int yy = Year(dt), mm = Month(dt), dd = Day(dt);
	bool toEom = preserve_eom && dd == DaysInMonth(yy, mm);
	const int ny = n_months / 12;
	yy += ny;
	mm += n_months - 12 * ny;
	if (mm > 12)
		mm -= 12, ++yy;
	if (mm < 1)
		mm += 12, --yy;
	const int dMax = DaysInMonth(yy, mm);
	if (toEom || dd > dMax)
		dd = dMax;
	return Date_(yy, mm, dd);
}

int operator-(const Date_& lhs, const Date_& rhs)
{
	return Date::ToExcel(lhs) - Date::ToExcel(rhs);
}

Date_ Date::Minimum()
{
	return FromExcel(EXCEL_OFFSET + 1);
}

Date_ Date::Maximum()
{
   return Minimum().AddDays(0xFFF0);
}

