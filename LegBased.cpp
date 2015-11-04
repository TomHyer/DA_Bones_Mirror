
#include "Platform.h"
#include "LegBased.h"
#include "Strict.h"

#include "Flow.h"
#include "Archive.h"

LegBased::Payout_& LegBased::Payout_::operator+=(const LegBased::Coupon_& c)
{
	coupons_.insert(make_pair(c.eventTime_, c));
	return *this;
}

void LegBased::Payout_::DoNode
	(const UpdateToken_& vls,
	State_* state,
	NodeValues_&dst)
const
{
	auto tt = coupons_.equal_range(vls.eventTime_);
	for (auto pc = tt.first; pc != tt.second; ++pc)
	{
		const Coupon_& c = pc->second;
		dst[c.pay_] += c.dcf_ * (*c.rate_)(vls);
	}
}

const LegBased::MakeRate_& operator+(const Handle_<LegBased::MakeRate_>& mr)
{
	static const LegBased::MakeRate_ DEFVAL;
	return mr.Empty() ? DEFVAL : *mr;
}

LegBased::MakeCoupon_::~MakeCoupon_()
{	}

LegBased::MakeCoupon_::MakeCoupon_
	(ValueRequest_& v,
	 const Handle_<MakeRate_>& r,
	 const String_&tn,
	 const Ccy_& ccy)
:
valueRequest_(v), makeRate_(r), tradeName_(tn), payCcy_(ccy)
{	}

LegBased::Coupon_ LegBased::MakeCoupon_::operator()
	(const LegPeriod_& period)
const
{
	Coupon_ ret;
	ret.dcf_ = period.accrual_->dcf_;
	Payment_ pay;
	pay.tag_.period_ = *period.accrual_;
	pay.tag_.description_ = "Coupon payment";
	pay.ccy_ = payCcy_;
	pay.date_ = period.payDate_;
	pay.stream_ = tradeName_;
	tie(pay.eventTime_, ret.rate_) = (+makeRate_)(valueRequest_, *period.rate_);
	ret.eventTime_ = pay.tag_.knownTime_ = pay.eventTime_;
	ret.pay_ = valueRequest_.PayDst(pay);
	return ret;
}

pair<DateTime_, Handle_<TradeAmount_> > LegBased::MakeRate_::operator()
	(ValueRequest_& request,
	const CouponRate_& rate)
const
{
	using namespace TradeAmount;
	if (auto fixed = dynamic_cast<const FixedRate_*>(&rate))
	{
		return make_pair(DateTime::Minimum(), Handle_<TradeAmount_>(new Deterministic_(fixed->rate_)));
	}
	return make_pair(DateTime_(), Handle_<TradeAmount_>());
}


