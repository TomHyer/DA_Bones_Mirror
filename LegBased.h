
// tools to build trades from legs and leg periods

#pragma once

#include "DateTime.h"
#include "Payment.h"
#include "TradeAmount.h"
#include "PayoutEuropean.h"
#include "Algorithms.h"
#include "ValueRequest.h"
#include "CouponRate.h"
#include "Period.h"


namespace LegBased
{
	struct Coupon_
	{
		DateTime_ eventTime_;
		Handle_<TradeAmount_> rate_;
		double dcf_;
		Handle_<Payment::Tag_> pay_;
	};

	class Payout_ : public PayoutForward_<PayoutSingle_<>>
	{
		// coupons indexed by own event time
		std::multimap<DateTime_, Coupon_> coupons_;
	public:
		Payout_(const String_& trade_name) : PayoutForward_<PayoutSingle_<>>(trade_name) {}

		Payout_& operator+=(const Coupon_& c);

		Vector_<DateTime_> EventTimes() const override
		{
			return Unique(Keys(coupons_));
		}

		void DoNode
			(const UpdateToken_& vls,
			 State_* state,
			 NodeValues_&dst)
		const override;
	};

	struct MakeRate_ : noncopyable
	{
		MakeRate_() {}

		virtual pair<DateTime_, Handle_<TradeAmount_> > operator()
			(ValueRequest_& request,
			const CouponRate_& rate)
		const;
	};

	struct MakeCoupon_
	{
		ValueRequest_& valueRequest_;
		const Handle_<MakeRate_> makeRate_;
		const String_ tradeName_;
		const Ccy_ payCcy_;

		MakeCoupon_
			(ValueRequest_& v, const Handle_<MakeRate_>& r,
			 const String_&tn, const Ccy_& ccy);
		virtual ~MakeCoupon_();

		virtual Coupon_ operator()
			(const LegPeriod_& period)
		const;
	};
}

const LegBased::MakeRate_& operator+(const Handle_<LegBased::MakeRate_>& mr);	// dereference or default to base class

