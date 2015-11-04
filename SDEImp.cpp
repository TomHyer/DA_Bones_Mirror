
#include "Platform.h"
#include "SDEImp.h"
#include <stack>
#include "Strict.h"

#include "Numerics.h"
#include "Exceptions.h"
#include "IndexParse.h"
#include "Swap.h"
#include "TradeAmount.h"
#include "Period.h"
#include "Conventions.h"

SDEImp::Update_::~Update_()
{	}

SDEImp::UpdateOne_::~UpdateOne_()
{	}

namespace
{
	template<class E_> struct Stack_ : std::stack<E_>
	{
		typedef typename std::stack<E_>::container_type container_t;
		// provide access to base
		const container_t& Peek() const { return std::stack<E_>::c; }
	};

	struct UpdateImp_ : SDEImp::Update_
	{
		Vector_<pair<Valuation::address_t, Handle_<SDEImp::UpdateOne_>>> updates_;

		void operator()
			(Vector_<>* vals,
			 const Vector_<>& state,
			 const UpdateToken_& pass)
		const override
		{
			assert(&pass[0] == &vals->front());
			for (const auto& u : updates_)
				(*vals)[u.first] = (*u.second)(state.begin(), pass);
		}
	};

	struct Continuation_
	{
		Handle_<Index_> index_;
		Handle_<SDEImp::UpdateOne_> update_;
		Vector_<Handle_<Index_> > wait_;
		Continuation_(const Handle_<Index_>& index,
			const Handle_<SDEImp::UpdateOne_>& update,
			const Vector_<Handle_<Index_> >& wait)
			: index_(index), update_(update), wait_(wait) {}
	};

	Handle_<Index_> FindAnother
		(const std::map<IndexKey_, Valuation::address_t>& all,
		 const std::set<IndexKey_>& done)
	{
		assert(all.size() > done.size());
		auto pa = done.empty()
			? all.begin()
			: all.upper_bound(*done.rbegin());
      if (pa == all.end())
      {
         pa = all.begin();   // fast method failed
         auto pd = done.begin();
         while (pd != done.end() && pa->first == *pd)
         {
            ++pa, ++pd;
            assert(pa != all.end());
         }
      }
		return pa->first.val_;
	}

	bool IsCircular(const std::deque<Continuation_>& tbc)
	{
		for (auto pTop = tbc.begin(); pTop != tbc.end(); ++pTop)
			for (auto pBot = tbc.begin(); pBot != pTop; ++pBot)
				if (LinearSearch(pTop->wait_, pBot->index_))
					return true;	// bottom should wait for top of stack to be done, not vice versa
		return false;
	}
}	// leave local

Valuation::address_t RequestAtTime_::operator()
	(const Index_& index)
{
	// make the key we need
	IndexKey_ key(Index::Clone(index));
	if (!done_.count(key))
	{   // we don't have this update yet
		wait_.push_back(key.val_);
	}
	if (!all_.count(key))
	{
		// we haven't seen this request yet
		stale_ = true;
	}
	return request_.Fixing(t_, index);
}

SDEImp::Update_* SDEImp_::NewUpdate
	(_ENV, ValueRequestImp_& requests,
	 const DateTime_& event_time)
const
{
	std::unique_ptr<UpdateImp_> retval(new UpdateImp_);
	Stack_<Continuation_> tbc;
	RequestAtTime_ toDo(requests, event_time);
	for (;;)
	{
		Handle_<SDEImp::UpdateOne_> update;
		Handle_<Index_> next;
		if (tbc.empty())
		{   // no continuations
			toDo.Refresh();
			if (toDo.all_.size() == toDo.done_.size())
				break;
			next = FindAnother(toDo.all_, toDo.done_);
		}
		else if (tbc.top().wait_.empty())
		{   // now ready for this one
			next = tbc.top().index_;
			update = tbc.top().update_;
			tbc.pop();
		}
		else
		{   // still clearing the decks
			next = tbc.top().wait_.back();
			tbc.top().wait_.pop_back();
		}

		// get an update, and see whether we have to wait
		assert(toDo.wait_.empty());
		if (update.Empty())
			update.reset(NewUpdateOne(_env, toDo, *next));

		if (toDo.wait_.empty())
		{   // we can do it now
			toDo.Push(&retval->updates_, next, update);
		}
		else
		{   // put it on hold, do others first
			tbc.push(Continuation_(next, update, toDo.wait_));
			assert(!IsCircular(tbc.Peek()));
			toDo.wait_.clear();
		}
	}
	return retval.release();
}

void RequestAtTime_::Refresh()
{
	if (stale_)
		all_ = request_.AtTime(t_);
	stale_ = false;
}

void RequestAtTime_::Push
	(Vector_<pair<Valuation::address_t, update_t> >* updates,
	 const IndexKey_& index, 
	 const update_t& update)
{
	updates->emplace_back(all_[index], update);
	done_.insert(index);
}

SDEImp::UpdateOne_* SDEImp::AsUpdate(const Handle_<TradeAmount_>& amt)
{
	struct Mine_ : UpdateOne_
	{
		Handle_<TradeAmount_> amt_;
		Mine_(const Handle_<TradeAmount_>& amt) : amt_(amt) {}
		double operator()
			(const Vector_<>::const_iterator&,	
			 const UpdateToken_& prior)
		const 
		{
			return (*amt_)(prior);
		}
	};
	return new Mine_(amt);
}


namespace
{
//----------------------------------------------------------------------------
//
// Swap updater, based on DF and Libor requests to the model
//
	struct PVAccumulator_
	{
		Vector_<>::const_iterator state_;
		const UpdateToken_& prior_;
		PVAccumulator_(const Vector_<>::const_iterator& state, const UpdateToken_& prior) : state_(state), prior_(prior) {}
		double operator()(double sofar, const Handle_<SDEImp::UpdateOne_>& pv) const
		{
			return sofar + (*pv)(state_, prior_);
		}
	};

	// this code is somewhat different from that for valuation of legs (LegBased.cpp)
		// there, we show the payments explicitly and let the ValuesStore handle discounting
		// here we need the leg PVs immediately, so we make DF requests that let us handle discounting
	struct UpdateSwapRate_ : SDEImp::UpdateOne_
	{
		Vector_<Handle_<UpdateOne_>> fixedPV_;	// at unit notional and unit coupon
		Vector_<Handle_<UpdateOne_>> floatPV_;	// at unit notional

		UpdateSwapRate_(const Vector_<Handle_<UpdateOne_>>& fixed_pvs, const Vector_<Handle_<UpdateOne_>>& float_pvs) : fixedPV_(fixed_pvs), floatPV_(float_pvs) {}

		double operator()
			(const Vector_<>::const_iterator& state,
			 const UpdateToken_& prior)
		const override
		{
			PVAccumulator_ accumulator(state, prior);
			const double fixedPV = Accumulate2(fixedPV_, accumulator);
			const double floatPV = Accumulate2(floatPV_, accumulator);
			REQUIRE(!IsZero(fixedPV), "Fixed swap leg has zero value");
			return floatPV / fixedPV;
		}
	};

	struct ActivateLegPeriod_
	{
		RequestAtTime_& model_;
		const Ccy_& ccy_;
		ActivateLegPeriod_(RequestAtTime_& model, const Ccy_& ccy) : model_(model), ccy_(ccy) {}

		Handle_<SDEImp::UpdateOne_> operator()(const Handle_<LegPeriod_>& period) const
		{
			Index::DF_ df(ccy_, period->payDate_);
			auto dfVal = TradeAmount::AsAmount(model_(df));
			auto product = TradeAmount::Product(dfVal)(period->accrual_->dcf_);		// curried partial product
			if (auto fixed = handle_cast<FixedRate_>(period->rate_))
			{	
				Handle_<TradeAmount_> amount(product(fixed->rate_).NewAmount());
				return SDEImp::AsUpdate(amount);
			}
			if (auto libor = handle_cast<LiborRate_>(period->rate_))
			{
				Index::Libor_ index(ccy_, libor->rate_, Libor::StartFromFix(ccy_, libor->fixDate_.Date()));
				Handle_<TradeAmount_> amount(product(TradeAmount::AsAmount(model_(index))).NewAmount());
				return SDEImp::AsUpdate(amount);
			}
			THROW("Inrecognized rate type in leg period");
		}
	};
}	// leave local

SDEImp::UpdateOne_* SDEImp::NewSwapUpdater
	(RequestAtTime_& t,
	 const Index::Swap_& swap)
{
	const Date_ start = swap.StartDate(t.t_);
	auto fixedLeg = Swap::FixedSide(swap.ccy_, start, swap.tenor_, 1.0, 1.0);
	auto floatLeg = Swap::FloatSide(swap.ccy_, start, swap.tenor_, 1.0);
	ActivateLegPeriod_ activate(t, swap.ccy_);
	return new UpdateSwapRate_(Apply(activate, fixedLeg), Apply(activate, floatLeg));
}
