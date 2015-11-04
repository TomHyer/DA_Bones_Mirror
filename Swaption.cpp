
#include "Platform.h"
#include "Swaption.h"
#include "Strict.h"

#include "LegBased.h"
#include "IndexIr.h"

namespace
{
	struct MakeForecast_ : LegBased::MakeRate_
	{
		DateTime_ expiry_;
		MakeForecast_(const DateTime_& expiry) : expiry_(expiry) {}
		pair<DateTime_, Handle_<TradeAmount_> > operator()
			(ValueRequest_& req, const CouponRate_& rate) const
		{
			if (auto libor = dynamic_cast<const LiborRate_*>(&rate))
			{
				Index::Libor_ index(libor->ccy_, libor->rate_);
				return make_pair(expiry_, TradeAmount::AsAmount(req.Fixing(expiry_, index)));
			}
			return LegBased::MakeRate_::operator()(req, rate);
		}
	};

	struct ToDiscountedValue_
	{
		ValueRequest_& valueRequest_;
		const String_ tradeName_;
		const Ccy_ payCcy_;
		const MakeForecast_ makeRate_;

		TradeAmount_* operator()(const LegPeriod_& pd) const
		{
			Index::DF_ df(payCcy_, pd.payDate_);
			Valuation::address_t dfLoc = valueRequest_.Fixing(makeRate_.expiry_, df);
			auto product = TradeAmount::Product
					(makeRate_(valueRequest_, *pd.rate_).second)
					(pd.accrual_->dcf_)
					(TradeAmount::AsAmount(dfLoc));
			return product.NewAmount();
		}
	};

	struct SwaptionPayout_ : PayoutSimple_
	{
		int sign_;   // buy/sell
		Vector_<Handle_<TradeAmount_> > underlyings_;
		dst_t payDst_;

      SwaptionPayout_(const String_& name, const DateTime_& expiry, int sign,
         const Vector_<Handle_<TradeAmount_> >& underlyings,
         const dst_t& pay_dst)
         :
         PayoutSimple_(name, expiry), sign_(sign), underlyings_(underlyings), payDst_(pay_dst)
      {}

		void DoNode
			(const UpdateToken_& values,
			 State_*,
			 NodeValues_& pay)
		const override
		{
			assert(values.eventTime_ == eventTime_);
			double pv = 0.0;
			for (const auto& u : underlyings_)
				pv += (*u)(values);
         pay[payDst_] += sign_ > 0.0 ? Max(0.0, pv) : Min(0.0, pv);
		}
	};
}

