
// implementation tools for ValueRequest
	// trades should not include this

#pragma once

#include "ValueRequest.h"

struct IndexKey_;

class IndexPathHistory_ : noncopyable
{
public:
	virtual ~IndexPathHistory_();
	virtual DateTime_ ResetTime() const = 0;
	virtual Handle_<IndexPathHistorical_> History
		(const Index_& index)
	const = 0;
};

class ValueRequestImp_ : public ValueRequest_
{
public:
	virtual address_t InsertFixing(double amount) = 0;

	virtual IndexAddress_ IndexPath
		(const DateTime_& last_event_time,
		 const Index_& index,
		 const IndexPathHistory_* historical) = 0;
	IndexAddress_ IndexPath
		(const DateTime_& last_event_time,
		 const Index_& index)
	{
		return IndexPath(last_event_time, index, 0);
	}

	virtual IndexAddress_ InsertPath
		(const Handle_<IndexPath_>& path) = 0;

	virtual std::map<IndexKey_, address_t> AtTime
		(const DateTime_& t) const = 0;
};
