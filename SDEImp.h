
// supporting classes/functions for use inside SDE implementations

#pragma once

#include "Vectors.h"
#include "SDE.h"
#include "ValueModel.h"
#include "IndexIr.h"

class UpdateToken_; 
class TradeAmount_;

namespace SDEImp
{
	class Update_ : noncopyable
	{
	public:
		virtual ~Update_();
		virtual void operator()
			(Vector_<>* vals,
			const Vector_<>& state,
			const UpdateToken_& pass)
		const = 0;
	};

	class UpdateOne_ : noncopyable
	{
	public:
		virtual ~UpdateOne_();
		virtual double operator()
			(const Vector_<>::const_iterator& state,	// can't move the iterator or use it to write
			 const UpdateToken_& prior)
		const = 0;
	};
}

class RequestAtTime_
{
	ValueRequestImp_& request_;
	std::map<IndexKey_, Valuation::address_t> all_;
	bool stale_;         // if all_ needs updating
	std::set<IndexKey_> done_;
	Vector_<Handle_<Index_> > wait_;   // priors

	friend class SDEImp_;
	typedef Handle_<SDEImp::UpdateOne_> update_t;
	RequestAtTime_(ValueRequestImp_& r, const DateTime_& t) : request_(r), all_(r.AtTime(t)), stale_(false), t_(t)
	{   }

	void Refresh();
	void Push
		(Vector_<pair<Valuation::address_t, update_t> >* updates,
		 const IndexKey_& index, 
		 const update_t& update);

public:
	const DateTime_ t_;
	Valuation::address_t operator()(const Index_& index);
};


// helps implementations implement the SDE_ interface
class SDEImp_ : public SDE_
{
public:
	SDEImp::Update_* NewUpdate
		(_ENV, ValueRequestImp_& requests,
		const DateTime_& event_time)
	const;

	virtual SDEImp::UpdateOne_* NewUpdateOne
		(_ENV, RequestAtTime_& t,
		const Index_& index)
	const = 0;

	IndexPath_* NewIndexPath
		(const Index_& index,
		const Vector_<DateTime_>& index_times,
		ValueRequest_& request);
};

struct IndexIsSimilar_ : noncopyable
{
	virtual bool operator()(const Index_& index) const = 0;
};

namespace SDEImp
{
	// promote a TradeAmount_ (which does not see the state directly) to an Updater
	UpdateOne_* AsUpdate(const Handle_<TradeAmount_>& amt);

	// build a swap updater out of libor/df updaters
	UpdateOne_* NewSwapUpdater
		(RequestAtTime_& t,
		 const Index::Swap_& swap);
}

