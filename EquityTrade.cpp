
#include "Platform.h"
#include "EquityTrade.h"
#include "Strict.h"

#include "PayoutEuropean.h"
#include "AssetValue.h"
#include "Trade.h"
#include "IndexEquity.h"

namespace
{
	struct EquityForwardPayout_ : PayoutSimple_
	{
		Valuation::address_t fixing_;
		double strike_, size_;   // size is signed
		dst_t dst_;

      EquityForwardPayout_
         (const String_& name, const DateTime_& expiry,
         Valuation::address_t fixing, double strike,
         double size, const Handle_<Payment::Tag_>& dst)
         :
         PayoutSimple_(name, expiry), fixing_(fixing), strike_(strike), size_(size), dst_(dst) {}

		void DoNode
			(const UpdateToken_& values,
			 State_*,
			 NodeValues_& pay)
		const
		{
			assert(values.eventTime_ == eventTime_);
			const double spot = values[fixing_];
			pay[dst_] += size_ * (spot - strike_);
		}
	};

	Underlying_ EquityUnderlying
		(const Ccy_& ccy,
		 const Handle_<Index::Equity_>& index,
		 const DateTime_& expiry)
	{
		Underlying_ retval;
		retval.payCcys_[ccy] = expiry.Date();
		retval.indices_[IndexKey_(handle_cast<Index_>(index))] = expiry;
		return retval;
	}

	Payment_ MakePayment
		(const DateTime_& expiry,
		 const Ccy_& ccy,
		 const String_& stream)
	{
		Payment::Info_ info("Equity forward delivery", expiry);
		return Payment_(expiry, ccy, expiry.Date(), stream, info);
	}

	struct EquityForward_ : Trade_
	{
		Handle_<Index::Equity_> index_;
		DateTime_ expiry_;
		double strike_;
		double size_;	// signed number of contracts
	
		EquityForward_
			(const String_& trade_name,
			 const Handle_<Index::Equity_>& index,	
			 const Ccy_& ccy,
			 const DateTime_& expiry,
			 double strike,
			 int signed_num_contracts,
			 const CollateralType_& collateral)
		:
		Trade_(Vector::V1(trade_name), EquityUnderlying(ccy, index, expiry), ccy, collateral),
		index_(index),
		expiry_(expiry),
		strike_(strike),
		size_(signed_num_contracts)
		{   }

		Payout_* MakePayout
			(const ValuationParameters_&,
			 ValueRequest_& mkt)
		const override
		{
			const String_& name = valueNames_[0];
			Handle_<Payment::Tag_> payDst(mkt.PayDst(MakePayment(expiry_, valueCcy_, name)));
			return new EquityForwardPayout_
					(name, expiry_, mkt.Fixing(expiry_, *index_),	strike_, size_, payDst);
		}
	};
}