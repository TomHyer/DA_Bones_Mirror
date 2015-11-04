
// historical fixings available per index

#pragma once

#include <map>
#include "DateTime.h"
#include "Storable.h"

class FixHistory_
{
public:
	typedef std::map<DateTime_, double> vals_t;
private:
	vals_t vals_;
public:
	FixHistory_(const vals_t& vals) : vals_(vals) {}
	double Find(const DateTime_& fix_time, bool quiet = false) const;
};

namespace FixHistory
{
	const FixHistory_& Empty();
}

class Fixings_ : public Storable_
{
public:
	typedef std::map<DateTime_, double> vals_t;
	const vals_t vals_;

	Fixings_
		(const String_& index_name,
		 const vals_t& vals = vals_t())
	:
	Storable_("Fixings", index_name), vals_(vals) {}
};

