
#pragma once

#include <memory>
#include <utility>
#include <assert.h>
#include <boost/noncopyable.hpp>

#ifdef INFINITY
#undef INFINITY
#endif

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#pragma warning(disable:4996)

using boost::noncopyable;
using std::pair;
using std::make_pair;
template<class T> using scoped_ptr = const std::unique_ptr<T>;
// whenever a class takes a default template type, the compiler insists on seeing the default exactly once
	// thus the default must be here, or else in the header
		// thus anything we want to forward-declare, we must forward-declare here
template<class E_ = double> class Vector_;
template<class E_ = double> class Matrix_;
template<class E_ = double> class SquareMatrix_;
class Dictionary_;

namespace DA
{
	static const double EPSILON = 2e-14;
	static const double INFINITY = 1e29;
	static const double PI = 3.1415926535897932;
}
inline bool IsZero(double x) { return x < DA::EPSILON && -x < DA::EPSILON; }
inline bool IsPositive(double x) { return x >= DA::EPSILON; }
inline bool IsNegative(double x) { return x <= -DA::EPSILON; }

template<class T_> T_ Square(const T_& x) { return x * x; }
template<class T_> T_ Cube(const T_& x) { return x * x * x; }
template<class T_> T_ Max(const T_& a, const T_& b) { return a > b ? a : b; }
template<class T_> T_ Min(const T_& a, const T_& b) { return a < b ? a : b; }
struct Empty_ {};

template<class T_> class Handle_ : public std::shared_ptr<const T_>
{
	typedef typename std::shared_ptr<const T_> base_t;
public:
	Handle_() : base_t() {}
	Handle_(const T_* src) : base_t(src) {}
	Handle_(const base_t& src) : base_t(src) {}
	bool Empty() const { return !base_t::get(); }
};

template<class T_, class U_> Handle_<T_> handle_cast(const std::shared_ptr<U_>& src)
{
	return Handle_<T_>(std::dynamic_pointer_cast<const T_>(src));
}

template<class T_, class... Args > Handle_<T_> make_handle(Args&&... args)
{
	return std::make_shared<const T_>(std::forward<Args>(args)...);
}

template<typename T_> bool TruthValueOf(const T_& src) { return !!src; }
template<typename T_> bool TruthValueOf(const Handle_<T_>& src) { return !src.Empty(); }	// maybe not needed -- std::shared_ptr seems to implicitly convert to bool (which I don't like)
template<typename T_> double NumericValueOf(const T_& src) { return double(src); }

#define XXRUN_AT_LOAD(c, u1, u2) namespace{struct __run##u1##u2{__run##u1##u2(){c;}}; static const __run##u1##u2 runAtLoad##u1##u2;}	// can't consume a semicolon after the macro, because we need to wrap in a local namespace
#define XRUN_AT_LOAD(c, u1, u2) XXRUN_AT_LOAD(c, u1, u2)
#define RUN_AT_LOAD(code) XRUN_AT_LOAD(code, __COUNTER__, __LINE__)
#define RETURN_STATIC(...) static __VA_ARGS__ RETVAL; return RETVAL  // variadic in case type contains commas

#define VALUE_TYPE_OF(expr) typename std::remove_const<typename std::remove_reference<decltype(expr)>::type>::type