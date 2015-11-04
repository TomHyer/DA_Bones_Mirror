
// collection of trades to be handled en bloc

#pragma once

#include "Trade.h"
#include "ValuationMethod.h"

/*IF--------------------------------------------------------------------------
settings TradeBookkeeping
	Non-financial characteristics of a trade
is_record
&members
risk_class is string
	The book or risk class name
-IF-------------------------------------------------------------------------*/
#include "MG_TradeBookkeeping_object.h"

/*IF--------------------------------------------------------------------------
storable PortfolioEntry
	Trade and its associated information inside a portfolio
version 1
&members
trade is handle TradeData
valuation_params is settings ValuationParameters
bookkeeping is settings TradeBookkeeping
-IF-------------------------------------------------------------------------*/
#include "MG_PortfolioEntry_object.h"

struct PortfolioEntry_ : Storable_
{
	const Handle_<TradeData_> trade_;
	const ValuationParameters_ params_;
	const TradeBookkeeping_ books_;

	PortfolioEntry_(const Handle_<TradeData_>& trade, const ValuationParameters_& params, const TradeBookkeeping_& books);
	void Write(Archive::Store_& dst) const override;
};

/*IF--------------------------------------------------------------------------
storable Portfolio
	Set of trades, valuation instructions, bookkeeping info
&members
name is ?string
vals is +handle PortfolioEntry
-IF-------------------------------------------------------------------------*/

class Portfolio_ : public Storable_
{
	Vector_<Handle_<PortfolioEntry_>> vals_;
public:
	Portfolio_
		(const String_& name, 
		 const Vector_<Handle_<TradeData_>>& trades,
		 const Vector_<TradeBookkeeping_>& books,
		 const Vector_<ValuationParameters_>& params);
	Portfolio_(const String_& name,
		 const Vector_<Handle_<PortfolioEntry_>>& vals);

	void Write(Archive::Store_& dst) const override;

	int NTrades() const { return vals_.size(); }		// number of trades, not necessarily of values
	Handle_<Trade_> Trade(int i_trade) const;			// parse on demand 
	void Clear(int i_trade) const;						// un-parse if risk task needs to free memory
	const ValuationParameters_& ValuationParams
		(int i_trade) const;

	Vector_<String_> BookkeepingLabels() const;			// labels must correspond to return values of Bookkeeping() above
	Vector_<Cell_> Bookkeeping(int i_trade) const;		// does not need to include valueNames -- risk task sees those, we don't
};
