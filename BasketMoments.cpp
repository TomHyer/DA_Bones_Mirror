
#include "Platform.h"
#include "BasketMoments.h"
#include "Strict.h"

#include "Semianalytic.h"
#include "YC.h"

/*
// should be located with model
double OptionValue
	(const Date_& valuation,
	 const YieldCurve_& yc,
	 double s_0,
	 const Dividends_& divs,
	 const PiecewiseConstant_& vols,
	 const DateTime_& expiry,
	 const Date_& delivery,
	 double strike,
	 const OptionType_& type)
{
	const Vector_<> moments = MomentsOfExponent(AccumulateMoments(vols, divs, expiry, 3));
	assert(moments.size() >= 4);   // 0, 1, 2, 3
	double shift, shiftedFwd, shiftedVol;
	FitThreeMomentsSLN
		(moments[1], moments[2], moments[3], &shift, &shiftedFwd, &shiftedVol);
	return Distribution::Black_(shiftedFwd, shiftedVol, 1.0).OptionPrice(strike - shift, type)
			* yc.Discount(CollateralType_())(valuation, delivery);
}

// deprecated, fat interface
struct PriceByMoments_ : SemianalyticPricer_
{
	bool Attempt
		(_ENV, const Trade_& trade,
		 const Model_& model,
		 const Valuation::Parameters_& params,
		 Vector_<pair<String_, double> >* vals)
	const
	{
		scoped_ptr<EquityOptionData_> opt(EquityOption::NewData(trade));
		DYN_PTR(myModel, const BlackWithDividends_, &model);
		if (!myModel || !opt.get())
			return false;
		const String_& eq = opt->eqName_;
		const double value = OptionValue
				(*model.YieldCurve(trade.valueCcy_), myModel->Spot(eq), myModel->Dividends(eq),
				 myModel->Vols(eq), opt->expiry_, opt->delivery_, opt->strike_, opt->type_);
		vals->push_back(make_pair(trade.valueNames_[0], value));
		return true;
	}
};

*/

