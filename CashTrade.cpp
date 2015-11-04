
#include "Platform.h"
#include "CashTrade.h"
#include "Strict.h"

#include "Trade.h"
#include "Flow.h"
#include "PayoutEuropean.h"
#include "AssetValue.h"
#include "Archive.h"

using Vector::V1;

/*IF--------------------------------------------------------------------------
storable CashTradeData
	Deterministic cashflows only
version 1
&members
name is ?string
	Name of the trade
ccy is enum Ccy
	Currency of cashflows
payDates is date[]
	Dates on which cashflows are paid
amounts is number[]
	Cashflow amounts corresponding to dates
collateral is enum CollateralType
&conditions
payDates.size() == amounts.size()
	Must have one cashflow amount for each payment date
-IF-------------------------------------------------------------------------*/

namespace
{
	static const auto ZipToFlow = [](const Date_& dt, double amt) { return Flow_({ dt, amt }); };
	static const auto FlowDate = [](const Flow_& f) { return f.payDate_; };
	static const auto FlowAmount = [](const Flow_& f) { return f.amount_; };

	Underlying_ CashUnderlying
		(const Ccy_& ccy,
		 const Vector_<Flow_>& flows)
	{
		Underlying_ retval;
		retval.payCcys_[ccy] = *MaxElement(Apply(FlowDate, flows));
		return retval;
	}

	struct CashPayout_ : PayoutSimple_ 
	{
		Vector_<pair<dst_t, double> > flows_;
      CashPayout_(const String_& name) : PayoutSimple_(name, DateTime::Minimum()) {}

		void DoNode
			(const UpdateToken_&,
			 State_*,
			 NodeValues_& pay_dst)
		const
		{
			for (const auto& f : flows_)
				pay_dst[f.first] += f.second;
		}
	};

	Payment_ MakePayment
		(const String_& stream,
		 const Ccy_& ccy,
		 const Flow_& flow)
	{
		static const DateTime_ WHEN = DateTime::Minimum();
		return Payment_(WHEN, ccy, flow.payDate_, stream, Payment::Info_("Contractual cashflow", WHEN));
	}

	struct CashTrade_ : Trade_, IsCashflows_
	{
		Vector_<Flow_> flows_;

		CashTrade_
			(const String_& name,
			 const Ccy_& ccy,
			 const Vector_<Flow_>& flows,
			 const CollateralType_& collateral)
		:
		Trade_(V1(name), CashUnderlying(ccy, flows), ccy, collateral),
		flows_(flows)
		{   }

      Payout_* MakePayout
         (const ValuationParameters_&,
          ValueRequest_& value_request)
       const override;

	  Vector_<Flow_> Flows(const Ccy_& ccy) const override
	  {
		  return ccy == underlying_.payCcys_.begin()->first	
				? flows_
				: Vector_<Flow_>();
	  }
	};

   Payout_* CashTrade_::MakePayout
      (const ValuationParameters_&,
       ValueRequest_& value_request)
   const
   {
      const String_& name = valueNames_[0];
      std::unique_ptr<CashPayout_> retval(new CashPayout_(name));
	  for (const auto& f : flows_)
      {
         Payout_::dst_t tag = value_request.PayDst(MakePayment(name, valueCcy_, f));
         if (tag != Payment::Null())
            retval->flows_.push_back(make_pair(tag, f.amount_));
      }
      return retval.release();
   }


	// trade is only defined locally
#include "MG_CashTradeData_v1_Write.inc"

	struct CashTradeData_ : TradeData_
	{
		Ccy_ ccy_;
		Vector_<Date_> dates_;
		Vector_<> amounts_;
		CollateralType_ collateral_;

		Trade_* XParse() const override
		{
			return new CashTrade_
				(name_, ccy_, Apply(ZipToFlow, dates_, amounts_), collateral_);
		}

		void Write(Archive::Store_& dst) const override
		{
         CashTradeData_v1::XWrite(dst, name_, ccy_, dates_, amounts_, collateral_);
		}

		CashTradeData_
			(const String_& name,
			const Ccy_& ccy,
			const Vector_<Date_>& pay_dates,
			const Vector_<>& amounts,
			const CollateralType_& collateral)
		:
		TradeData_(name),
		ccy_(ccy),
		dates_(pay_dates),
		amounts_(amounts),
		collateral_(collateral)
		{	}
	};

#include "MG_CashTradeData_v1_Read.inc"
}	// leave local

TradeData_* NewCashTrade
	(const String_& name,
	 const Ccy_& ccy,
	 const Vector_<Flow_>& flows,
	 const CollateralType_& collateral)
{
	return new CashTradeData_(name, ccy, Apply(FlowDate, flows), Apply(FlowAmount, flows), collateral);
}

Trade_* NewCashTradeImp
	(const String_& name,
	 const Ccy_& ccy,
	 const Vector_<Flow_>& flows,
	 const CollateralType_& collateral)
{
	return new CashTrade_(name, ccy, flows, collateral);
}


