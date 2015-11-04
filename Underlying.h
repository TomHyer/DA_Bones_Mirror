
// description of underlyings of a derivative trade

#pragma once

#include <map>
#include "Index.h"
#include "DateTime.h"
#include "Currency.h"

struct Underlying_
{
	class Parent_ : noncopyable
	{
	public:
		virtual ~Parent_();
	};
	Handle_<Parent_> parent_;

	std::map<Ccy_, Date_> payCcys_;
	std::map<IndexKey_, DateTime_> indices_;
	std::map<String_, Date_> credits_;

	Underlying_& operator+=(const Underlying_& more);
	// also have a facility to add individual events
	Underlying_& Include(const Ccy_& ccy, const Date_& pay_date);
	Underlying_& Include(const IndexKey_& index, const DateTime_& fix_date);
	Underlying_& Include(const String_& ref_name, const Date_& pay_date);
};


