
// base class for Index, which represents anything that generates a time series

#pragma once

#include "Environment.h"
#include "Strings.h"

class DateTime_;

namespace Index
{
	double PastFixing
		(_ENV, const String_& index_name,
		 const DateTime_& fixing_time,
		 bool quiet = false);
}

class Index_ : noncopyable
{
public:
	virtual ~Index_();
	virtual String_ Name() const = 0;
	virtual double Fixing(_ENV, const DateTime_& fixing_time) const;
};

struct IndexKey_
{
	const Handle_<Index_> val_;
	const String_ name_;	// precomputed

	// allow empty handle, but not implicitly
	IndexKey_(const Handle_<Index_>& val)
	: 
	val_(val),
	name_(val.Empty() ? String_() : val->Name())
	{   }

	const Index_* operator->() const { return val_.get(); }
};
inline bool operator<(const IndexKey_& lhs, const IndexKey_& rhs)
{
	return lhs.name_ < rhs.name_;	// postponed -- profile to see if this is a hotspot
}
inline bool operator==(const IndexKey_& lhs, const IndexKey_& rhs)
{
	return lhs.name_ == rhs.name_;
}

