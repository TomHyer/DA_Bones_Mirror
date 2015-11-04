
// simple trade -- set of deterministic cashflows
#pragma once

class String_;
struct Flow_;
class Ccy_;
class Trade_;
class TradeData_;
class CollateralType_;

TradeData_* NewCashTrade
	(const String_& name,
	 const Ccy_& ccy,
	 const Vector_<Flow_>& flows,
	 const CollateralType_& collateral);

Trade_* NewCashTradeImp
	(const String_& name,
	 const Ccy_& ccy,
	 const Vector_<Flow_>& flows,
	 const CollateralType_& collateral);

// query interface to let the semianalytic pricer recover the flows
struct IsCashflows_
{
	virtual Vector_<Flow_> Flows(const Ccy_& ccy) const = 0;
};