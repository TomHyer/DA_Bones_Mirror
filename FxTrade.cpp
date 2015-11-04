
#include "Platform.h"
#include "FxTrade.h"
#include "Strict.h"

#include "PayoutEuropean.h"
#include "ReceivePay.h"
#include "OptionType.h"
#include "BackwardInduction.h"

/*IF--------------------------------------------------------------------------
storable FxOption
	Option to exchange currencies
version 1
&members
name is ?string
	Name of the trade
dom_ccy is enum Ccy
dom_amt is number
	Domestic notional, positive
fgn_ccy is enum Ccy
fgn_amt is number
	Foreign notional, positive
rec_pay_fgn is enum RecPay
	Determines whether foreign notional is received or paid at exercise
expiry is datetime
	Time at which FX spot is observed and exercise decided
delivery is ?date
	Date on which FX is paid, if not deduced from expiry
&conditions
dom_amt > 0.0
fgn_amt > 0.0
-IF-------------------------------------------------------------------------*/

namespace
{
	template<class T_> struct FxPayout_ : PayoutSimple_	// template parameter calcs payments
	{
		Valuation::address_t fixing_;
		dst_t domDst_, fgnDst_;
		T_ payAmounts_;	// must take spot and return a pair (dom_pay_amt, fgn_pay_amt)

		FxPayout_
			(const String_& name,
			 const DateTime_& expiry,
			 Valuation::address_t fixing,
			 const Handle_<Payment::Tag_>& dom_dst,
			 const Handle_<Payment::Tag_>& fgn_dst,
			 const T_& pay_amount_calc)
		:
		PayoutSimple_(name, expiry),
		fixing_(fixing),
		domDst_(dom_dst),
		fgnDst_(fgn_dst),
      payAmounts_(pay_amount_calc)
		{	}

		void DoNode
			(const UpdateToken_& values,
			 State_*,
			 NodeValues_& pay)
		const
		{
			assert(values.eventTime_ == eventTime_);
			const double spot = values[fixing_];
			auto payDomFgn = payAmounts_(spot);
         if (!IsZero(payDomFgn.first))
			   pay[domDst_] += payDomFgn.first;
         if (!IsZero(payDomFgn.second))
			   pay[fgnDst_] += payDomFgn.second;
		}
	};

	template<bool CASH = false> struct ExecuteForward_
	{
      double domAmt_, fgnAmt_;
		ExecuteForward_(double dom_notional, double fgn_notional, const RecPay_& rp_fgn) : domAmt_(-dom_notional * rp_fgn.RecSign()), fgnAmt_(fgn_notional * rp_fgn.RecSign()) {}
		pair<double, double> operator()
			(double spot)
		const
		{
			return CASH
					? make_pair(domAmt_ + fgnAmt_ * spot, 0.0)
					: make_pair(domAmt_, fgnAmt_);
		}
	};

	template<bool CASH = false> struct ExecuteOption_
	{
      double domAmt_, fgnAmt_;
      double itmLev_, otmLev_;	// in [-1, 1] -- used to identify long/short call/put/straddle
		ExecuteOption_(double dom_notional, double fgn_notional, const RecPay_& rp_fgn, const OptionType_& cps, int option_sign) 
			:
      domAmt_(-dom_notional * rp_fgn.RecSign()),
      fgnAmt_(fgn_notional * rp_fgn.RecSign()),
		itmLev_(option_sign * cps.Payout(1.0, 0.0)),
		otmLev_(-option_sign * cps.Payout(0.0, 1.0))
		{	}

		pair<double, double> operator()
			(double spot)
		const
		{
			const double intrinsic = domAmt_ + fgnAmt_ * spot;
         const double leverage = intrinsic > 0.0 ? itmLev_ : otmLev_;
			return CASH
					? make_pair(leverage * intrinsic, 0.0)
					: make_pair(leverage * domAmt_, leverage * fgnAmt_);
		}
	};

	struct FxOptionPayout_AMC_ : PayoutSingle_<>
	{
      FxPayout_<ExecuteOption_<>> underlying_;
		int sign_;
		amount_t spotDst_;   // observable for AMC

      template<typename... Args_> FxOptionPayout_AMC_(const String_& name, int sign, amount_t spot_dst, Args_&&... underlying) 
         : 
      PayoutSingle_(name), 
      underlying_(std::forward<Args_>(underlying)...),
      sign_(sign),
      spotDst_(spot_dst)
      {  }

		Vector_<DateTime_> EventTimes() const { return underlying_.EventTimes(); }

		void DoNode
			(const UpdateToken_& values,
			 State_* state,
			 NodeValues_& pay_dst)
		const
		{
			underlying_.DoNode(values, state, pay_dst);		
			pay_dst[spotDst_] = values[underlying_.fixing_];
		}

		Vector_<BackwardInduction::Action_> BackwardSteps()	const
		{
			BackwardInduction::Action_ retval(underlying_.name_, underlying_.eventTime_, underlying_.eventTime_.Date());
			BackwardInduction::Exercise_ bermEx;
			bermEx.sign_ = sign_;
			// allow exercise to observe the spot where DoNode() stored it
			bermEx.observables_.push_back(spotDst_);
			retval.details_ = bermEx;
			return Vector::V1(retval);
		}
	};
}

//ExecuteForward_<> EF(0.0, 0.0, RecPay_("Rec"));
//FxPayout_<ExecuteForward_<>> FXF(String_(), DateTime_(), Valuation::address_t(), Handle_<Payment::Tag_>(), Handle_<Payment::Tag_>(), EF);
//ExecuteOption_<> EO(0.0, 0.0, RecPay_(), OptionType_(), 1);
//FxPayout_<ExecuteOption_<>> FXO(String_(), DateTime_(), Valuation::address_t(), Handle_<Payment::Tag_>(), Handle_<Payment::Tag_>(), EO);
//FxOptionPayout_AMC_ FXAO(String_(), -1, Handle_<Payment::Amount::Tag_>(), String_(), DateTime_(), Valuation::address_t(), Handle_<Payment::Tag_>(), Handle_<Payment::Tag_>(), EO);