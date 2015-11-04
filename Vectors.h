
#pragma once

#include <vector>

template<class E_> class Vector_ : private std::vector<E_>	// default is supplied in Platform.h
{
	typedef std::vector<E_> base_t;
public:
	Vector_() : base_t() {}
	Vector_(int size) : base_t(size) {}
	Vector_(size_t size) : base_t(size) {}
	Vector_(int size, const E_& fill) : base_t(size, fill) {}
	template<class I_> Vector_(I_ start, I_ stop) : base_t(start, stop) {}
	Vector_(const std::initializer_list<E_>& args) : base_t(args) {}

	void Swap(Vector_<E_>* other) { base_t::swap(*other); }
	void Fill(const E_& val) { std::fill(begin(), end(), val); }
	void Resize(int new_size) { base_t::resize(new_size, E_()); }

	template<class T_> void operator*=(const T_& scale) { std::transform(begin(), end(), begin(), std::bind2nd(std::multiplies<E_>(), scale)); }
	template<class T_> void operator+=(const T_& shift) { std::transform(begin(), end(), begin(), std::bind2nd(std::plus<E_>(), shift)); }
   template<class T_> void operator-=(const T_& shift) { std::transform(begin(), end(), begin(), std::bind2nd(std::minus<E_>(), shift)); }
   template<class T_> void operator+=(const Vector_<T_>& other) { std::transform(begin(), end(), other.begin(), begin(), std::plus<E_>()); }
   template<class T_> void operator-=(const Vector_<T_>& other) { std::transform(begin(), end(), other.begin(), begin(), std::minus<E_>()); }
   template<class I_> void Assign(I_ begin, I_ end) { base_t::assign(begin, end); }
	template<class I_> void Append(I_ begin, I_ end) { base_t::insert(end(), begin, end); }
	template<class C_> void Append(const C_& other) { base_t::insert(end(), other.begin(), other.end()); }
	bool operator==(const Vector_<E_>& other) const;
	bool operator!=(const Vector_<E_>& other) const;

	// reveal std::vector types (Clang requires std:: qualifier and explicit typename)
	using typename std::vector<E_>::iterator;
	using typename std::vector<E_>::const_iterator;
	using typename std::vector<E_>::reference;
	using typename std::vector<E_>::const_reference;
	using typename std::vector<E_>::value_type;
	// reveal std::vector functionality
	using std::vector<E_>::begin;
	using std::vector<E_>::end;
	using std::vector<E_>::rbegin;
	using std::vector<E_>::rend;
	using std::vector<E_>::front;
	using std::vector<E_>::back;
	int size() const { return static_cast<int>(std::vector<E_>::size()); }
	using std::vector<E_>::empty;
	using std::vector<E_>::erase;
	using std::vector<E_>::push_back;
	using std::vector<E_>::pop_back;
	using std::vector<E_>::operator[];
	using std::vector<E_>::reserve;
	using std::vector<E_>::clear;

	// emplace_back is a special case; because it is not part of std::vector<bool>, we have to explicitly forward
	template<class... _Valty> void emplace_back(_Valty&&... _Val) { base_t::emplace_back(_Val...); }
};

namespace Vector
{
	template<class C1_, class C2_> bool EqualElements(const C1_& lhs, const C2_& rhs)
	{
		assert(lhs.size() == rhs.size());
		for (auto pl = lhs.begin(), pr = rhs.begin(); pl != lhs.end(); ++pl, ++pr)
		if (*pl != *pr)
			return false;
		return true;
	}
	template<class E_> bool Equal(const Vector_<E_>& lhs, const Vector_<E_>& rhs)
	{
		return lhs.size() == rhs.size()
			&& Vector::EqualElements(lhs, rhs);
	}

	template<class E_> Vector_<E_> V1(const E_& val)
	{
		return Vector_<E_>(1, val);
	}

	template<class E_> Vector_<E_> Join(const E_& e1, const E_& e2)
	{
		auto retval = V1(e1);
		retval.push_back(e2);
		return retval;
	}
	template<class E_> Vector_<E_> Join(const Vector_<E_>& v, const E_& more)
	{
		auto retval = v;
		retval.push_back(more);
		return retval;
	}

	// partial specialization of container Join (see Algorithms.h) for Vector_, which lacks insert
	template<class E1_, class C2_> Vector_<E1_> Join(const Vector_<E1_>& c1, const C2_& c2)
	{
		auto retval = c1;
		retval.Append(c2.begin(), c2.end());
		return retval;
	}
	Vector_<int> UpTo(int n);
}

template<class E_> bool Vector_<E_>::operator==(const Vector_<E_>& rhs) const { return Vector::Equal(*this, rhs); }
template<class E_> bool Vector_<E_>::operator!=(const Vector_<E_>& rhs) const { return !Vector::Equal(*this, rhs); }

