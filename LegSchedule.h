
// full leg schedules (see LegParams.h for generating parameters)

#pragma once

#include "Vectors.h"

class String_;
class LegScheduleParams_;
class RecPay_;
class NotionalExchange_;
class Holidays_;
struct LegPeriod_;
class Ccy_;

namespace LegBuild
{
	Vector_<LegPeriod_> Fixed
		(const Ccy_& ccy,
		 const LegScheduleParams_& schedule,
		 const RecPay_& rec_pay,
		 const Vector_<>& notional,
		 const Vector_<>& coupon,
		 const NotionalExchange_* exchange = nullptr);

	Vector_<LegPeriod_> Libor
		(const Ccy_& ccy,
		 const LegScheduleParams_& schedule,
		 const RecPay_& rec_pay,
		 const Vector_<>& notional,
		 const Vector_<>& margin = Vector_<>(),
		 const Holidays_* fixing_holidays = nullptr,
		 const Vector_<int>& fixing_delay = Vector_<int>(),
		 const NotionalExchange_* exchange = nullptr);
}

