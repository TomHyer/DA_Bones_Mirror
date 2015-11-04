
// container-level algorithms, mostly based on STL iterator algorithms

#pragma once

#include <algorithm>
#include <numeric>
#include <type_traits>

template<class T> struct vector_of
{
	typedef typename ::Vector_<typename std::remove_reference<typename std::remove_const<T>::type>::type> type;
};

template<class CS_, class CD_> void Copy(const CS_& src, CD_* dst)
{
	assert(dst && src.size() == dst->size());
	std::copy(src.begin(), src.end(), dst->begin());
}

template<class C_> Vector_<typename C_::value_type> Copy(const C_& src)
{
	typedef Vector_<typename C_::value_type> vector_t;
	vector_t retval(src.size());
	std::copy(src.begin(), src.end(), retval.begin());
	return retval;
}

template<class CS_, class OP_, class CD_> void Transform(const CS_& src, OP_ op, CD_* dst)
{
	assert(dst && src.size() == dst->size());
	std::transform(src.begin(), src.end(), dst->begin(), op);
}

template<class CS1_, class CS2_, class OP_, class CD_> void Transform(const CS1_& src1, const CS2_& src2, OP_ op, CD_* dst)
{
	assert(dst && src1.size() == dst->size() && src2.size() == dst->size());
	std::transform(src1.begin(), src1.end(), src2.begin(), dst->begin(), op);
}

template<class C_, class OP_> void Transform(C_* to_change, OP_ op)
{
	assert(to_change != nullptr);
	std::transform(to_change->begin(), to_change->end(), to_change->begin(), op);
}

template<class C_, class CI_, class OP_> void Transform(C_* to_change, const CI_& other, OP_ op)
{
	assert(to_change != nullptr);
	std::transform(to_change->begin(), to_change->end(), other.begin(), to_change->begin(), op);
}

template<class C_, class E_> void Fill(C_* range, const E_& val)
{
	std::fill(range->begin(), range->end(), val);
}

template <typename C, typename OP> auto Apply(OP op, const C& src)->typename vector_of<decltype(op(*src.begin()))>::type
{
	typedef typename vector_of<decltype(op(*src.begin()))>::type vector_t;
	vector_t retval(src.size());
	Transform(src, op, &retval);
	return retval;
}

template<class C1_, class C2_, class OP_> auto Apply(OP_ op, const C1_& src1, const C2_& src2)->typename vector_of<decltype(op(*src1.begin(), *src2.begin()))>::type
{
	typedef typename vector_of<decltype(op(*src1.begin(), *src2.begin()))>::type vector_t;
	vector_t retval(src1.size());
	Transform(src1, src2, op, &retval);
	return retval;
}

template<class C1_, class C2_> void Append(C1_* c1, const C2_& c2)
{
	c1->insert(c1->end(), c2.begin(), c2.end());
}
// specialization for Vector_ which does not expose insert
template<class E1_, class C2_> void Append(Vector_<E1_>* c1, const C2_& c2)
{
	c1->Append(c2);
}

template<class C1_, class C2_> C1_ Concatenate(const C1_& c1, const C2_& c2)
{
	C1_ retval = c1;
	Append(&retval, c2);
	return retval;
}

template<class C_, class P_> C_ Filter(const C_& src, const P_& pred)
{
   typedef decltype(*src.begin()) element_t;
	C_ retval(src);
   auto stop = std::remove_if(retval.begin(), retval.end(), [&](const element_t& e){return !pred(e); });
	retval.erase(stop, retval.end());
	return retval;
}

template<class C_, class LT_> C_ Unique(const C_& src, const LT_& less)
{
   C_ retval(src);
   std::sort(retval.begin(), retval.end(), less);
   retval.erase(unique(retval.begin(), retval.end()), retval.end());
   return retval;
}
template<class C_> C_ Unique(const C_& src)
{
   return Unique(src, std::less<typename C_::value_type>());
}

template<class C_> typename C_::const_iterator LowerBound(const C_& c, const typename C_::value_type& x)
{
   return std::lower_bound(c.begin(), c.end(), x);
}
template<class C_> typename C_::const_iterator UpperBound(const C_& c, const typename C_::value_type& x)
{
   return std::upper_bound(c.begin(), c.end(), x);
}

template<class C_, class E_> bool BinarySearch(const C_& c, const E_& val)
{
	return std::binary_search(c.begin(), c.end(), val);
}
template<class C_, class E_> bool LinearSearch(const C_& c, const E_& val)
{
	return std::find(c.begin(), c.end(), val) != c.end();
}

template<class C_, class L_> bool IsMonotonic(const C_& c, const L_& pred)
{
	if (c.size() < 2)
		return true;
	auto pvs = c.begin();
	for (auto nxt = Next(pvs); nxt != c.end(); pvs = nxt++)
	{
		if (!pred(*pvs, *nxt))
			return false;
	}
	return true;
}
template<class C_> bool IsMonotonic(const C_& c)
{
	return IsMonotonic(c, std::less<typename C_::value_type>());
}

// zipping
template<class C1_, class C2_> auto Zip(const C1_& v1, const C2_& v2) -> Vector_<decltype(make_pair(v1.front(), v2.front()))>
{
   auto join = [](const typename C1_::value_type& e1, const typename C2_::value_type& e2) {return make_pair(e1, e2); };
   return Apply(join, v1, v2);
}

template<class C_> typename vector_of<typename C_::key_type>::type Keys(const C_& src)
{
	typedef typename vector_of<typename C_::key_type>::type vector_t;
	auto op = [](const C_::value_type& kv){return kv.first; };
	vector_t retval(src.size());
	Transform(src, op, &retval);
	return retval;
}

template<class C_> typename ::Vector_<typename C_::mapped_type> MapValues(const C_& src)
{
	typedef typename vector_of<typename C_::mapped_type>::type vector_t;
	auto op = [](const C_::value_type& kv){return kv.second; };
	Vector_<typename C_::mapped_type> retval(src.size());
	Transform(src, op, &retval);
	return retval;
}

template<class T_> T_ Next(const T_& src) { T_ retval(src);  return ++retval; }
template<class T_> T_ Previous(const T_& src) { T_ retval(src);  return --retval; }
#define ASSIGN(p, v) if (!p); else *p = v
#define DEREFERENCE(p, v) (p ? *p : v)

template<class C_> typename C_::const_iterator MaxElement(const C_& src)
{
	return std::max_element(src.begin(), src.end());
}
template<class C_> typename C_::const_iterator MinElement(const C_& src)
{
	return std::min_element(src.begin(), src.end());
}

// helper to get an offset which is always an int (not ptrdiff_t which may be 64-bit)
template<class C_> struct ToLoc_
{
	const C_& src_;
	ToLoc_(const C_& src) : src_(src) {}
	template<class I_> int operator()(const I_& it) const { return static_cast<int>(it - src_.begin()); }
};
template<class C_> ToLoc_<C_> ToLoc(const C_& src) { return ToLoc_<C_>(src); }

template<class C_> void Sort(C_* vals)
{
	std::sort(vals->begin(), vals->end());
}
template<class C_, class P_> void Sort(C_* vals, const P_& pred)
{
	std::sort(vals->begin(), vals->end(), pred);
}

template<class C_, class P_> bool AllOf(const C_& vals, const P_& pred)
{
	return std::all_of(vals.begin(), vals.end(), pred);
}

