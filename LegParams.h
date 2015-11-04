
// parameters for leg schedules and rates

#pragma once

#include "Storable.h"
#include "Date.h"
#include "Optionals.h"
#include "Vectors.h"

struct Cell_;
template<class T_> class EnumDict_;

/*IF--------------------------------------------------------------------------
enumeration NotionalExchange
	Identifies when/whether to exchange notionals in a swap
switchable
alternative NONE
alternative START_AND_END BOTH YES
	Notional exchange at both start and end of swap
alternative END_ONLY END
	Notional exchange at end, not at start
alternative START_ONLY START
	Present for completeness, maybe not used in the real world
-IF-------------------------------------------------------------------------*/
#include "MG_NotionalExchange_enum.h"	

/*IF--------------------------------------------------------------------------
enumeration ScheduleParameter
	Anything that can be overridden in a schedule
switchable
alternative NAME
	Schedules can be given a name
alternative COUPON_PERIOD
alternative DAY_BASIS
alternative ROLL_DAY
	Day of month
alternative ROLL_DIRECTION
alternative ROLL_SPECIAL
alternative STUB_AT_END
alternative LONG_COUPON
alternative PAY_UPFRONT
alternative PAY_HOLIDAYS
-IF-------------------------------------------------------------------------*/
#include "MG_ScheduleParameter_enum.h"	

/*IF--------------------------------------------------------------------------
storable LegScheduleParams
	Parameters for the leg coupon and accrual dates
version 1
&members
name is ?string
	Name of the storable object
startDate is date
matDate is ?date
	Fixed maturity date, if specified directly
tenor is ?string
	Leg tenor to determine maturity, if not specified directly
couponPeriod is ?string
	A date increment string
dayBasis is ?string
	The daycount method for computing coupon amounts
rollDay is ?integer
	A fixed day of the month for rolls, if specified
rollDirection is ?string
rollSpecial is ?string
	E.g., for IMM rolls
stubAtEnd is ?boolean
longCoupon is ?boolean
payUpfront is ?boolean
payHolidays is ?string
payDelay is *integer
	A single offset (number of business days) for all payments, or a vector of one entry per payment
&conditions
matDate_.IsValid() == tenor_.empty()\Must specify either maturity date or tenor, not both
-IF-------------------------------------------------------------------------*/

class LegScheduleParams_ : public Storable_
{
public:
	Date_ startDate_;
	Date_ matDate_;
	String_ tenor_;
	String_ couponPeriod_;
	String_ dayBasis_;
	boost::optional<int> rollDay_;
	String_ rollDirection_;
	String_ rollSpecial_;
	boost::optional<bool> stubAtEnd_;
	boost::optional<bool> longCoupon_;
	boost::optional<bool> payUpfront_;
	String_ payHolidays_;
	Vector_<int> payDelay_;

	LegScheduleParams_(const String_& name) : Storable_("LegScheduleParams", name) {}
	LegScheduleParams_(const String_& name, const Date_& startDate, const boost::optional<Date_>& matDate, const String_& tenor, const String_& couponPeriod, const String_& dayBasis, const boost::optional<int>& rollDay, const String_& rollDirection, const String_& rollSpecial, const boost::optional<bool>& stubAtEnd, const boost::optional<bool>& longCoupon, const boost::optional<bool>& payUpfront, const String_& payHolidays, const Vector_<int>& payDelay) : Storable_("LegScheduleParams", name), startDate_(startDate), matDate_(+matDate), tenor_(tenor), couponPeriod_(couponPeriod), dayBasis_(dayBasis), rollDay_(rollDay), rollDirection_(rollDirection), rollSpecial_(rollSpecial), stubAtEnd_(stubAtEnd), longCoupon_(longCoupon), payUpfront_(payUpfront), payHolidays_(payHolidays), payDelay_(payDelay) {}

	void Write(Archive::Store_& dst) const override;
};

Handle_<LegScheduleParams_> ScheduleWithOverrides
	(const Date_& start_date,
	 const Cell_& maturity,
	 const EnumDict_<ScheduleParameter_>& overrides);
