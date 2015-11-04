
#include "Platform.h"
#include "CouponRate.h"
#include "Strict.h"

#include "Exceptions.h"
#include "PeriodLength.h"
#include "CollateralType.h"

#include "MG_TradedRate_enum.inc"

PeriodLength_ TradedRate_::Period() const
{
	static const PeriodLength_ QUARTERLY("3M");
	static const PeriodLength_ SEMI("6M");
	switch (Switch())
	{
	case Value_::LIBOR_3M_CME:
	case Value_::LIBOR_3M_LCH:
	case Value_::LIBOR_3M_FUT:
		return QUARTERLY;

	case Value_::LIBOR_6M_CME:
	case Value_::LIBOR_6M_LCH:
		return SEMI;

	default:
		NOTICE(String());
		THROW("Can't find period length of traded rate");
	}
}

CouponRate_::~CouponRate_()
{	}

LiborRate_::LiborRate_(const DateTime_& fix_date, const Ccy_& ccy, const TradedRate_& rate) : fixDate_(fix_date), ccy_(ccy), rate_(rate) {}

Clearer_ TradedRate_::Clearer() const
{
	switch (Switch())
	{
	case Value_::LIBOR_3M_CME:
	case Value_::LIBOR_3M_FUT:
	case Value_::LIBOR_6M_CME:
		return Clearer_::Value_::CME;

	case Value_::LIBOR_3M_LCH:
	case Value_::LIBOR_6M_LCH:
		return Clearer_::Value_::LCH;
	}
	NOTICE(String());
	THROW("Can't find clearinghouse for traded rate");
}

TradedRate_ FindRate(const PeriodLength_& period, const Clearer_& clearer)
{
	switch (clearer.Switch())
	{
	case Clearer_::Value_::LCH:
		switch (period.Months())
		{
		case 3:
			return TradedRate_::Value_::LIBOR_3M_LCH;
		case 6:
			return TradedRate_::Value_::LIBOR_6M_LCH;
		}
		break;
	case Clearer_::Value_::CME:
		switch (period.Months())
		{
		case 3:
			return TradedRate_::Value_::LIBOR_3M_CME;
		case 6:
			return TradedRate_::Value_::LIBOR_6M_CME;
		}
		break;
	}
	THROW("Can't find traded rate for period/clearinghouse combination");
}

