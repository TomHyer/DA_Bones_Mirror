
#pragma once

#include <string>
template<class E_> class Vector_;
#define BAREWORD(word) static const String_ word(#word);

namespace
{
	// handwritten lookup table -- specifies ordering
	static const unsigned char CI_ORDER[128] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101 };
}
// traits for case-insensitive string
struct ci_traits : std::char_traits<char>
{
	typedef char _E;
	static inline unsigned char SortVal(const _E& _X)
	{
		unsigned char X(_X);
		return (X & 128) | CI_ORDER[X & 127];
	}
	static bool __cdecl eq(const _E& _X, const _E& _Y)
	{
		return SortVal(_X) == SortVal(_Y);
	}
	static bool __cdecl lt(const _E& _X, const _E& _Y)
	{
		return SortVal(_X) < SortVal(_Y);
	}
	static int compare(const _E* _P1, const _E* _P2, size_t _N) 
	{
		while (_N-- > 0)
		{
			if (SortVal(*_P1) < SortVal(*_P2))
				return -1;
			if (SortVal(*_P1) > SortVal(*_P2))
				return 1;
			++_P1, ++_P2;
		}
		return 0;
	}
	static const char* find(const _E* _P, size_t _N, const _E& _A) 
	{
		for (auto a = SortVal(_A); _N > 0 && SortVal(*_P) != a; ++_P, --_N);	// no loop body
		return _N > 0 ? _P : nullptr;
	}
};

// having String_ be a real class, not just a typedef, lets us forward-declare it
// at the expense of having to repeat all the constructors
// the inheritance, like that of Vector_, is safe as long as we add no data members
class String_ : public std::basic_string<char, ci_traits>
{
   typedef std::basic_string<char, ci_traits> base_t;
public:
	String_() {}
	String_(const char* src) : base_t(src) {}
	String_(const base_t& src) : base_t(src) {}
	String_(size_t size, char val) : base_t(size, val) {}
	template<class I_> String_(I_ begin, I_ end) : base_t(begin, end) {}
	// we can also add an implicit constructor from std::string
	String_(const std::string& src) : base_t(*reinterpret_cast<const String_*>(&src)) {}	// I hate calling c_str() unnecessarily
	// support our swap idiom
	void Swap(String_* other) { swap(*other); }
   // support an interface idiom
   const String_& get_value_or(const String_& other) const { return empty() ? other : *this; }
};
// we also have to disambiguate a lot of operators
inline bool operator==(const String_& lhs, const String_& rhs)
{
	return static_cast<const std::basic_string<char, ci_traits>&>(lhs) == static_cast<const std::basic_string<char, ci_traits>&>(rhs);
}
inline bool operator==(const String_& lhs, const std::basic_string<char, ci_traits>& rhs)
{
	return static_cast<const std::basic_string<char, ci_traits>&>(lhs) == rhs;
}
inline bool operator==(const std::basic_string<char, ci_traits>& lhs, const String_& rhs)
{
	return lhs == static_cast<const std::basic_string<char, ci_traits>&>(rhs);
}
inline bool operator==(const String_& lhs, const char* rhs)
{
	return static_cast<const std::basic_string<char, ci_traits>&>(lhs) == rhs;
}
inline bool operator==(const char* lhs, const String_& rhs)
{
	return lhs == static_cast<const std::basic_string<char, ci_traits>&>(rhs);
}

namespace String
{
	Vector_<String_> Split
		(const String_& src,
		char separator,
		bool keep_empties);

	bool IsNumber(const String_& src);
	double ToDouble(const String_& src);	
	int ToInt(const String_& src);
	String_ FromDouble(double src);
	String_ FromInt(int src);
	String_ Condensed(const String_& src);
	bool Equivalent(const String_& lhs, const char* rhs);

	String_ NextName(const String_& name);
   String_ Uniquifier(const void* p);

	template<class T_> String_ ToString(const T_& src)
	{
		return std::to_string(src).c_str();	// no efficient way to convert 
	}

	struct Joiner_
	{
		String_ sep_;
		bool skipEmpty_;
		Joiner_(const String_& sep, bool skip_empty = true) : sep_(sep), skipEmpty_(skip_empty) {}

		String_ operator()(const String_& sofar, const String_& more) const
		{
			if (sofar.empty())
				return more;
			if (more.empty() && skipEmpty_)
				return sofar;
			return sofar + sep_ + more;
		}
	};
	template<class C_> String_ Accumulate(const C_& vals, const String_& sep, bool skip_empty = true) 
	{ 
		return std::accumulate(vals.begin(), vals.end(), String_(), Joiner_(sep, skip_empty)); 
	}
}
// global forwarding functions
inline String_ ToString(int i) { return String::FromInt(i); }
