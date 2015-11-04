
// variant of a dictionary, with keys belonging to an enumeration
	// designed to work with Machinist enumerations

#pragma once

#include "Dictionary.h"

template<class T_> class EnumDict_
{
	std::map<T_, Cell_> vals_;
public:
	EnumDict_() {}
	EnumDict_(const Dictionary_& src)
	{
		for (const auto& k_v : src)
			Insert(T_(k_v.first), k_v.second);
	}

	void Insert(const T_& key, const Cell_& value)
	{
		REQUIRE(!vals_.count(key), "Duplicate insertion of " + String_(key.String()));
		vals_.emplace(key, value);
	}
	int Size() const { return static_cast<int>(vals_.size()); }
	bool Has(const T_& key) { return !!vals_.count(key); }
	const Cell_& At(const T_& key, bool optional = false) const
	{
		
		auto p = vals_.find(key);
		if (p == vals_.end())
		{
			REQUIRE(optional, "No entry for " + String_(key.String()));
			return Dictionary::BlankCell();
		}
		return p->second;
	}

	typename std::map<T_, Cell_>::const_iterator begin() const { return val_.begin(); }
	typename std::map<T_, Cell_>::const_iterator end() const { return val_.end(); }
};

