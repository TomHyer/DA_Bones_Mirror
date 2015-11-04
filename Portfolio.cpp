
#include "Platform.h"
#include "Portfolio.h"
#include "Strict.h"

#include "Exceptions.h"
#include "CellUtils.h"
#include "Archive.h"
#include "_Reader.h"	// levelization violation

using Vector::Join;

#include "MG_TradeBookkeeping_object.inc"
#include "MG_PortfolioEntry_v1_Write.inc"
#include "MG_PortfolioEntry_v1_Read.inc"
#include "MG_Portfolio_Write.inc"
#include "MG_Portfolio_Read.inc"

PortfolioEntry_::PortfolioEntry_(const Handle_<TradeData_>& trade, const ValuationParameters_& params, const TradeBookkeeping_& books)
	:
Storable_("PortfolioEntry", trade->name_),
trade_(trade),
params_(params),
books_(books)
{	}

void PortfolioEntry_::Write(Archive::Store_& dst) const
{
	PortfolioEntry_v1::XWrite(dst, trade_, params_, books_);
}

Portfolio_::Portfolio_
	(const String_& name,
	 const Vector_<Handle_<TradeData_>>& trades,
	 const Vector_<TradeBookkeeping_>& books,
	 const Vector_<ValuationParameters_>& params)
:
Storable_("Portfolio", name)
{
	const int n = trades.size();
	REQUIRE(books.size() == n, "Must have bookkeeping info for each trade");
	ValuationParameters_ vp((Dictionary_()));
	for (int ii = 0; ii < n; ++ii)
	{
		if (ii < params.size())
			vp = params[ii];
		vals_.emplace_back(new PortfolioEntry_(trades[ii], vp, books[ii]));
	}
}

Portfolio_::Portfolio_
	(const String_& name,
	 const Vector_<Handle_<PortfolioEntry_>>& trades)
:
Storable_("Portfolio", name),
vals_(trades)
{	}

void Portfolio_::Write(Archive::Store_& dst) const
{
	Portfolio::XWrite(dst, name_, vals_);
}

Handle_<Trade_> Portfolio_::Trade(int i_trade) const
{
	NOTICE(i_trade);
	REQUIRE(i_trade >= 0 && i_trade < NTrades(), "Invalid trade index");
	return vals_[i_trade]->trade_->Parse();
}
const ValuationParameters_& Portfolio_::ValuationParams(int i_trade) const
{	// we expect user has already gotten the trade
	return vals_[i_trade]->params_;
}

Vector_<String_> Portfolio_::BookkeepingLabels() const
{
	return Join(TradeBookkeeping_::VectorLabels(), String_("TradeName"));
}
Vector_<Cell_> Portfolio_::Bookkeeping(int i_trade) const
{	// we expect user has already gotten the trade
	return Join(vals_[i_trade]->books_.VectorContents(), Cell_(vals_[i_trade]->trade_->name_));
}

