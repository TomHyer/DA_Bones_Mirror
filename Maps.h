
// useful algorithms for maps

#pragma once

#include <map>

template<class C1_, class C2_> std::map<typename C1_::value_type, typename C2_::value_type> ZipToMap(const C1_& v1, const C2_& v2)
{
	assert(v1.size() == v2.size());
	std::map<typename C1_::value_type, typename C2_::value_type> retval;
	for (int ii = 0; ii < v1.size(); ++ii)
	{
		assert(!retval.count(v1[ii]));
		retval.insert(make_pair(v1[ii], v2[ii]));
	}
	return retval;
}

template<class C1_, class C2_> std::multimap<typename C1_::value_type, typename C2_::value_type> ZipToMultimap(const C1_& v1, const C2_& v2)
{
	assert(v1.size() == v2.size());
	std::multimap<typename C1_::value_type, typename C2_::value_type> retval;
	for (int ii = 0; ii < v1.size(); ++ii)
		retval.insert(make_pair(v1[ii], v2[ii]));
	return retval;
}

