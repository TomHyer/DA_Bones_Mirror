
// convenience wrappers for trades to access one fixing
	// TradeAmount provides operator() which takes the update token and returns the amount

#pragma once

#include "AssetValue.h"

class TradeAmount_ : noncopyable
{
public:
	virtual ~TradeAmount_();
	virtual double operator()(const UpdateToken_& values) const = 0;
};

namespace TradeAmount
{
	// degenerate case of a deterministic constant
	struct Deterministic_ : TradeAmount_
	{
		double val_;
		Deterministic_(double v) : val_(v) {}
		double operator()(const UpdateToken_&) const
		{
			return val_;
		}
	};

	struct Fixing_ : TradeAmount_
	{
		const Valuation::address_t loc_;
		Fixing_(Valuation::address_t p) : loc_(p) {}
		double operator()(const UpdateToken_& values) const
		{
			return values[loc_];
		}
	};
	inline Handle_<TradeAmount_> AsAmount(const Valuation::address_t& loc) { return new Fixing_(loc); }

	template<class OP_> struct Combined_ : TradeAmount_	// e.g., sums or products
	{
		Vector_<Handle_<TradeAmount_>> stochastic_;
		double deterministic_;
		Combined_(const Vector_<Handle_<TradeAmount_>>& s, double d) : stochastic_(s), deterministic_(d) {}
		// during pricing, get the final value
		double operator()(const UpdateToken_& values) const
		{
			double retval = deterministic_;
			for (const auto& s : stochastic_)
				retval = OP_()(retval, (*s)(values));
			return retval;
		}
	};

	// support for accretion of sums/products
	template<class OP_> struct Accumulate_
	{
		Vector_<Handle_<TradeAmount_>> stochastic_;
		double deterministic_;
		// seed with one thing
		Accumulate_(const Handle_<TradeAmount_>& s, double d) : deterministic_(d)
		{
			if (s)
				stochastic_.push_back(s);
		}

		Accumulate_<OP_> operator()(const Handle_<TradeAmount_>& s) 
		{
			Accumulate_<OP_> retval(*this); 
			retval.stochastic_.push_back(s); 
			return retval; 
		}
		Accumulate_<OP_> operator()(double d) 
		{ 
			Accumulate_<OP_> retval(*this); 
			retval.deterministic_ = OP_()(retval.deterministic_, d);
			return retval; 
		}

		// call when done accreting
		TradeAmount_* NewAmount() const { return new Combined_<OP_>(stochastic_, deterministic_); }
	};

	inline Accumulate_<std::plus<double>> Sum(const Handle_<TradeAmount_>& s = Handle_<TradeAmount_>()) 
	{
		return Accumulate_<std::plus<double>>(s, 0.0);
	}
	inline Accumulate_<std::multiplies<double>> Product(const Handle_<TradeAmount_>& s = Handle_<TradeAmount_>())
	{
		return Accumulate_<std::multiplies<double>>(s, 1.0);
	}
}
