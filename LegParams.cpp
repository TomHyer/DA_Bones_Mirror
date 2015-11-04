
#include "Platform.h"
#include "LegParams.h"
#include "Strict.h"

#include "Exceptions.h"
#include "Cell.h"
#include "EnumDict.h"
#include "Archive.h"

#include "MG_NotionalExchange_enum.inc"
#include "MG_ScheduleParameter_enum.inc"

#include "MG_LegScheduleParams_v1_Write.inc"
void LegScheduleParams_::Write(Archive::Store_& dst) const
{
	LegScheduleParams_v1::XWrite
			(dst, name_, startDate_, matDate_, tenor_, couponPeriod_, dayBasis_,
			 rollDay_, rollDirection_, rollSpecial_, stubAtEnd_, longCoupon_, 
			 payUpfront_, payHolidays_, payDelay_);
}
#include "MG_LegScheduleParams_v1_Read.inc"

namespace
{
	int SetOverride
		(String_* val,
		 const Cell_& src)
	{
		if (Cell::IsEmpty(src))
			return 0;
		REQUIRE(Cell::IsString(src), "Override is not a string");	// no coercion
		*val = Cell::OwnString(src);
		return 1;
	}
	int SetOverride
		(boost::optional<int>* val,
		 const Cell_& src)
	{
		if (Cell::IsEmpty(src))
			return 0;
		REQUIRE(Cell::IsInt(src), "Override is not a string");
		*val = Cell::ToInt(src);
		return 1;
	}
	int SetOverride
		(boost::optional<bool>* val,
		const Cell_& src)
	{
		if (Cell::IsEmpty(src))
			return 0;
		REQUIRE(Cell::IsBool(src), "Override is not a string");
		*val = Cell::ToBool(src);
		return 1;
	}
}	// leave local

Handle_<LegScheduleParams_> ScheduleWithOverrides
	(const Date_& start_date,
	 const Cell_& maturity,
	 const EnumDict_<ScheduleParameter_>& overrides)
{
	String_ name;
	int nUsed = 0;
	nUsed += SetOverride(&name, overrides.At(ScheduleParameter_::Value_::NAME, true));
	std::unique_ptr<LegScheduleParams_> retval(new LegScheduleParams_(name));
	retval->startDate_ = start_date;
	if (Cell::IsString(maturity))
		retval->tenor_ = Cell::OwnString(maturity);
	else
		retval->matDate_ = Cell::ToDate(maturity);
	nUsed += SetOverride(&retval->couponPeriod_, overrides.At(ScheduleParameter_::Value_::COUPON_PERIOD, true));
	nUsed += SetOverride(&retval->dayBasis_, overrides.At(ScheduleParameter_::Value_::DAY_BASIS, true));
	nUsed += SetOverride(&retval->rollDay_, overrides.At(ScheduleParameter_::Value_::ROLL_DAY, true));
	nUsed += SetOverride(&retval->rollDirection_, overrides.At(ScheduleParameter_::Value_::ROLL_DIRECTION, true));
	nUsed += SetOverride(&retval->rollSpecial_, overrides.At(ScheduleParameter_::Value_::ROLL_SPECIAL, true));
	nUsed += SetOverride(&retval->stubAtEnd_, overrides.At(ScheduleParameter_::Value_::STUB_AT_END, true));
	nUsed += SetOverride(&retval->longCoupon_, overrides.At(ScheduleParameter_::Value_::LONG_COUPON, true));
	nUsed += SetOverride(&retval->payUpfront_, overrides.At(ScheduleParameter_::Value_::PAY_UPFRONT, true));
	nUsed += SetOverride(&retval->payHolidays_, overrides.At(ScheduleParameter_::Value_::PAY_HOLIDAYS, true));
	// POSTPONED -- how to get pay delay in this interface?
	REQUIRE(nUsed == overrides.Size(), "There were unused overrides");	// POSTPONED -- be nicer to the user, tell him what he mis-spelled
	return retval.release();
}

