
// Black-Scholes distribution, option pricing, greeks

#pragma once

#include "Distribution.h"

namespace Distribution
{
	// compute forward option value in terms of forward price and deannualized vol -- no discounting
	double BlackOpt
		(double fwd,
		 double vol,
		 double strike,
		 const OptionType_& type);
	double BlackIV
		(double fwd,
		 double strike,
		 const OptionType_& type,
		 double price,
		 double guess = 0.0);	// start point of search, useful for deep-otm options
}

class DistributionBlack_ : public Distribution_
{
	double f_;
	double vol_;	// deannualized
public:
	DistributionBlack_(double fwd, double deann_vol) : f_(fwd), vol_(deann_vol) {}

	double Forward() const override { return f_; }
	double OptionPrice
		(double strike, const OptionType_& type)
	const override;

	double& Vol() override { return vol_; }
	const double& Vol() const override { return vol_; }
	double VolVega
		(double strike, const OptionType_& type)
	const override;
};