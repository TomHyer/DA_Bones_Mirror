
#include "Platform.h"
#include "TradeComposite.h"
#include "Strict.h"

#include "Algorithms.h"
#include "Functionals.h"
#include "Matrix.h"
#include "Composite.h"
#include "Strings.h"
#include "Exceptions.h"
#include "PayoutDecorate.h"
#include "BackwardInduction.h"
#include "Archive.h"
#include "Trade.h"

using std::map;

namespace
{	
/*IF--------------------------------------------------------------------------
storable CollectionTrade
	Collects values of other trades
version 1
manual
&members
name is ?string
   Object's name
trades is +handle TradeData
	All the trades which need to be valued for the composite
-IF-------------------------------------------------------------------------*/

	// collection trade requires a lot of machinery to ensure trades don't overlap
	struct XValuesForComposite_ : ValueRequest_
	{
		ValueRequest_& base_;
		String_ prefix_;

		XValuesForComposite_(ValueRequest_& vr) : base_(vr) {}

		Handle_<Payment::Tag_> PayDst
			(const Payment_& flow)
		{
			Payment_ temp(flow);
			temp.stream_ = prefix_ + temp.stream_;
			return base_.PayDst(temp);
		}

		Handle_<Payment::Default::Tag_> DefaultDst
			(const String_& stream)
		{
			return base_.DefaultDst(prefix_ + stream);
		}
		// ...
	};

	struct CollectionPayout_ : Composite_<const Payout_>
	{
		Vector_<Vector_<DateTime_> > eventTimes_;
		Vector_<String_> streamPrefixes_;

		// handle state with a Composite_   
		typedef Composite_<State_> state_t;
		State_* NewState() const
		{
			std::unique_ptr<state_t> retval(new state_t);
			for (const auto& t : contents_)
				retval->Append(t->NewState());
			return retval.release();
		};

		void StartPath(State_* _state) const
		{
			state_t& state = CoerceComposite(_state);
			assert(state.Size() == contents_.size());
			for (int it = 0; it < contents_.size(); ++it)
				contents_[it]->StartPath(state[it]);
		};

		void DoNode
			(const UpdateToken_& vls,
			 State_* _state,
			 NodeValues_& pay_to)
		const
		{
			state_t& myS = CoerceComposite(_state);
			assert(myS.Size() == contents_.size());
			const DateTime_& t = vls.eventTime_;
			for (int it = 0; it < contents_.size(); ++it)
			{
				if (BinarySearch(eventTimes_[it], t))
					contents_[it]->DoNode(vls, myS[it], pay_to);
			}
		}

		weights_t StreamWeights() const
		{
			weights_t retval;
			for (int it = 0; it < contents_.size(); ++it)
			{
				for (const auto& sub : contents_[it]->StreamWeights())
				{
					REQUIRE(!retval.count(sub.first), "Duplicate trade name in collection");
					auto prepend = [&](const pair<String_, double>& p)->pair<String_, double> { return make_pair(streamPrefixes_[it] + p.first, p.second); };
					retval[sub.first] = Apply(prepend, sub.second);
				}
			}
			return retval;
		}
	};

   Vector_<String_> AllValueNames(const Vector_<Handle_<Trade_>>& trades)
   {
      Vector_<String_> retval;
	  for (const auto& t : trades)
         retval.Append(t->valueNames_);
      return retval;
   }
   Underlying_ AllUnderlyings(const Vector_<Handle_<Trade_>>& trades)
   {
      Underlying_ retval;
	  for (const auto& t : trades)
         retval += t->underlying_;
      return retval;
   }
   CollateralType_ CollateralOfAll(const Vector_<Handle_<Trade_>>& trades)
   {
	   auto retval = trades[0]->collateral_;
	   for (const auto& t : trades)
		   REQUIRE(t->collateral_ == retval, "Can't make a composite trade across different collateral types");
	   // POSTPONED -- associate collateral with each payment, not with a whole trade?
	   return retval;
   }

   class CollectionTrade_ : public Composite_<const Trade_>, public IsCompositeTrade_
   {
      Vector_<Handle_<Trade_>> SubTrades() const override { return contents_; }
      Vector_<pair<String_, double>> FinalValues(const Vector_<pair<String_, double>>& component_vals) const override { return component_vals; }
   public:
      CollectionTrade_(const Ccy_& value_ccy, const Vector_<Handle_<Trade_>>& components) 
      : 
      Composite_<const Trade_>(components, AllValueNames(components), AllUnderlyings(components), value_ccy, CollateralOfAll(components))
      {  }

      Payout_* MakePayout(const ValuationParameters_&, ValueRequest_&) const override { return nullptr; }   // stubbed out, sorry
   };

#include "MG_CollectionTrade_v1_Write.inc"
   class CollectionTradeData_ : public Composite_<const TradeData_>
   {
   public:
      CollectionTradeData_(const String_& name, const Vector_<Handle_<TradeData_>>& components)
         :
      Composite_<const TradeData_>(components, name)
      {  }

      void Write(Archive::Store_& dst) const
      {
         CollectionTrade_v1::XWrite(dst, name_, contents_);
      }

      Trade_* XParse() const
      {
         auto parsed = Apply([](const Handle_<TradeData_>& t){ return t->Parse(); }, contents_);
         return new CollectionTrade_(parsed[0]->valueCcy_, parsed);
      }
   };
#include "MG_CollectionTrade_v1_Read.inc"

   Storable_* CollectionTrade_v1::Reader_::Build() const
   {
	   return new CollectionTradeData_(name_, trades_);
   }

/*IF--------------------------------------------------------------------------
storable RemappingTrade
	Sums, and optionally redirects, values of other trades
version 1
&members
name is ?string
	Name of the trade object
base is handle TradeData
	A trade (possibly composite) providing the input values
src_names is ?string[]
   If supplied, names of the source values which are to be used; otherwise all value_names of base trade are used
dst_names is ?string[]
	If supplied, names of the values produced by this trade; otherwise trade name is used
value_scalers is number[][]
	Coefficients in the linear combination, addressed as value_scaler[i_src][i_dst]
&conditions
value_scalers.Rows() == (src_names.empty() ? base->valueNames_ : src_names).size()
   Rows in linear mapping must correspond to src_names
value_scalers.Cols() == Max(1, dst_names.size())
   Columns in linear mapping must correspond to dst_names
-IF-------------------------------------------------------------------------*/

	// once we have the collection trade, the other kinds of composition just manipulate the mapping of stream to trade values
	struct LinearCombinationPayout_ : PayoutDecorated_
	{
		map<String_, map<String_, double>> coeffs_;
		LinearCombinationPayout_
			(const Handle_<Payout_>& base, 
			 const map<String_, map<String_, double>>& coeffs)
		: 
		PayoutDecorated_(base),
		coeffs_(coeffs)
		{	}

		weights_t StreamWeights() const
		{
			weights_t retval, base = base_->StreamWeights();
			for (const auto& srcCoeff : coeffs_)
			{
            NOTICE(srcCoeff.first);
            auto src = base.find(srcCoeff.first);
            REQUIRE(src != base.end(), "Source name not found in trades underlying linear combination");
			for (const auto& dstCoeff : srcCoeff.second)
               if (!IsZero(dstCoeff.second))
				   for (const auto& srcTerm : src->second)
                     retval[dstCoeff.first].emplace_back(srcTerm.first, dstCoeff.second * srcTerm.second);
            return retval;
			}
			return retval;
		}
	};

	struct TradeRemap_ : Trade_, IsCompositeTrade_
	{
		Handle_<Trade_> base_;			// itself probably a collection of trades
      Vector_<String_> srcNames_;   
      map<String_, map<String_, double>> remap_;

		TradeRemap_
			(const String_& name,
			 const Handle_<Trade_>& base,
			const Vector_<String_>& src_names,
			const Vector_<String_>& dst_names,
			const Matrix_<>& coeffs)
			 :
		Trade_(dst_names.empty() ? Vector::V1(name) : dst_names, base->underlying_, base->valueCcy_, base->collateral_),
		base_(base),
		srcNames_(src_names.empty() ? base->valueNames_ : src_names)
		{	
         // transform the coeffs into name-map form
         for (int ii = 0; ii < srcNames_.size(); ++ii)
         for (int jj = 0; jj < valueNames_.size(); ++jj)
         if (!IsZero(coeffs(ii, jj)))
            remap_[srcNames_[ii]][valueNames_[jj]] = coeffs(ii, jj);
      }

		Payout_* MakePayout
			(const ValuationParameters_& params,
			 ValueRequest_& model)
		const override
		{
         Handle_<Payout_> base(base_->MakePayout(params, model));
         return new LinearCombinationPayout_(base, remap_);
		}

      Vector_<Handle_<Trade_>> SubTrades() const override { return Vector::V1(base_); }
      Vector_<pair<String_, double>> FinalValues(const Vector_<pair<String_, double>>& component_vals) const override
      {
         map<String_, double> retval;
		 for (const auto& c : component_vals)
         {
            auto pr = remap_.find(c.first);
            if (pr != remap_.end())
				for (const auto& r : pr->second)
                  retval[r.first] += r.second * c.second;
         }
         return Zip(Keys(retval), MapValues(retval));
      }
   };

#include "MG_RemappingTrade_v1_Write.inc"
   class RemappingTrade_ : public TradeData_
   {
      Handle_<TradeData_> base_;
      Vector_<String_> srcNames_;
      Vector_<String_> dstNames_;
      Matrix_<> valueScalers_;

      void Write(Archive::Store_& dst) const 
      {
         RemappingTrade_v1::XWrite(dst, name_, base_, srcNames_, dstNames_, valueScalers_);
      }

      Trade_* XParse() const
      {
         return new TradeRemap_(name_, base_->Parse(), srcNames_, dstNames_, valueScalers_);
      }

   public:
      RemappingTrade_(const String_& name, const Handle_<TradeData_>& base, const Vector_<String_>& src_names, const Vector_<String_>& dst_names, const Matrix_<>& value_scalers)
         :
         TradeData_(name), base_(base), srcNames_(src_names), dstNames_(dst_names), valueScalers_(value_scalers) {}
   };
#include "MG_RemappingTrade_v1_Read.inc"
}  // leave local

TradeData_* Trade::NewSum
   (const String_& name,
    const Vector_<Handle_<TradeData_>>& trades)
{
   if (trades.size() > 1)
   {
      // collect into a composite and sum that
      Handle_<TradeData_> temp(new CollectionTradeData_(name, trades));
      return NewSum(name, Vector::V1(temp));
   }
   auto base = trades[0];
   Matrix_<> coeffs(base->Parse()->valueNames_.size(), 1);
   coeffs.Fill(1.0);
   return new RemappingTrade_(name, base, Vector_<String_>(), Vector_<String_>(), coeffs);
}

