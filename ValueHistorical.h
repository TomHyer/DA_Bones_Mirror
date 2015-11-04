
// common implementation tools to deal with the past, shared by different models of the future

#pragma once

#include "ValueModel.h"
#include "Environment.h"
#include "DateTime.h"
#include "IndexPath.h"
#include "Index.h"

// a decorator of ValueRequest_, which handles the past and ignores the future
class PastAwareRequest_ : public ValueRequestImp_
{
	ValueRequestImp_& model_;	// supplied by the SDE_ -- handles requests in the future
	const Environment_& env_;   // or other fixings access
	DateTime_ resetTime_;
	Date_ accountingDate_;

	// implement the construction of a single historical path
	Handle_<IndexPathHistorical_> HistoricalPath
		(const Index_& index,
		 const Handle_<IndexPathHistorical_>& prior);

	struct History_ : IndexPathHistory_
	{
		PastAwareRequest_* parent_;
		const IndexPathHistory_* base_;
		History_(PastAwareRequest_* p, const IndexPathHistory_* b) : parent_(p), base_(b) {}

		DateTime_ ResetTime() const
		{
			return parent_->resetTime_;
		}
		Handle_<IndexPathHistorical_> History
			(const Index_& index)
		const
		{
			auto prior = base_
				? base_->History(index)
				: Handle_<IndexPathHistorical_>();
			return parent_->HistoricalPath(index, prior);
		}
	};

	Handle_<Payment::Tag_> PayDst
		(const Payment_& flow)
	{
		return flow.date_ < accountingDate_
			? Payment::Null()
			: model_.PayDst(flow);
	}

	Handle_<Payment::Default::Tag_> DefaultDst
		(const String_& stream)
	{
		return model_.DefaultDst(stream);
	}

	address_t Fixing
		(const DateTime_& event,
		 const Index_& index)
	{
		return event < resetTime_
			? model_.InsertFixing(index.Fixing(0, event))
			: model_.Fixing(event, index);
	}
	address_t InsertFixing(double fixing)
	{
		return model_.InsertFixing(fixing);
	}

	IndexAddress_ IndexPath
		(const DateTime_& last_event_time,
		const Index_& index)
	{
		History_ h(this, 0);
		return last_event_time < resetTime_
			? model_.InsertPath(handle_cast<IndexPath_>(h.History(index)))
			: model_.IndexPath(last_event_time, index, &h);
	}
	IndexAddress_ IndexPath
		(const DateTime_& last_event,
		 const Index_& index,
		 const IndexPathHistory_* historical)
	{
		History_ h(this, historical);
		return model_.IndexPath(last_event, index, &h);
	}
	IndexAddress_ InsertPath
		(const Handle_<IndexPath_>& path)
	{
		return model_.InsertPath(path);
	}
};

