
#include "Platform.h"
#include "_Cookie.h"
#include <map>
#include <boost/variant.hpp>
#include "Strict.h"

#include "Exceptions.h"

#ifdef THREADED_FFI
#include <mutex>
static std::mutex TheObjectsMutex;
#define LOCK_OBJECTS std::lock_guard<std::mutex> l(TheObjectsMutex)
#else
#define LOCK_OBJECTS
#endif

void HandleDelete(void* ph)
{
	// ph must be a pointer-to-handle
	if (ph)
		delete reinterpret_cast<Handle_<Storable_>*>(ph);
}

namespace
{
	// the main design consideration for stashable objects is to avoid copying input arrays twice
	struct Stashable_
	{
		typedef boost::variant<Handle_<Storable_>, Handle_<Vector_<>>, Handle_<Vector_<int>>, Handle_<String_>> variant_t;
		std::shared_ptr<variant_t> val_;

		Stashable_() {}
		Stashable_(const Handle_<Storable_>& h) : val_(new variant_t(h)) {}
		Stashable_(const Handle_<Vector_<>>& v) : val_(new variant_t(v)) {}
		Stashable_(const Handle_<Vector_<int>>& v) : val_(new variant_t(v)) {}
		Stashable_(const Handle_<String_>& s) : val_(new variant_t(s)) {}
	};

	// a cookie is a key into this static map
	std::map<int, Stashable_>& TheStash()
	{
      RETURN_STATIC(std::map<int, Stashable_>);
	}

	int Unique()
	{
		// call after LOCK_OBJECTS
		static int RETVAL = 0;
		return ++RETVAL;
	}

	template<class T_> int ToCookie(const T_& stashable_value)
	{
		LOCK_OBJECTS;	
		const int retval = Unique();
		TheStash()[retval] = Stashable_(stashable_value);
		return retval;
	}

	template<class T_> int CopyToCookie(const T_& val)
	{
		return ToCookie(Handle_<T_>(new T_(val)));
	}

	const Stashable_& StashFind(int cookie)
	{
		static const Stashable_ EMPTY;
		LOCK_OBJECTS;
		auto pc = TheStash().find(cookie);
		if (pc == TheStash().end())
			return EMPTY;
		return pc->second;
	}
	void StashDrop(int cookie)
	{
		LOCK_OBJECTS;
		TheStash().erase(cookie);
	}

	// a biscuit is just a list of cookies, or else an error
	struct Biscuit_
	{
		Vector_<int> cookies_;
		String_ error_;
	};
	std::map<double, Biscuit_>& TheBiscuits()
	{
		RETURN_STATIC(std::map<double, Biscuit_>);
	}

	// suporting functions for extraction
	struct VectorSize_ : boost::static_visitor<int>
	{
		template<class E_> int operator()(const Handle_<Vector_<E_>>& src) const { return src->size(); }
		int operator()(...) const { return -1; }
	};

	template<class E_> struct VectorBase_ : boost::static_visitor<const E_*>
	{
		const E_* operator()(const Handle_<Vector_<E_>>& src) const { return &(*src)[0]; }
		const E_* operator()(...) const { return nullptr; }
	};
}	// leave local

Cookie::Output_::Output_()
{
	static const double OFFSET = (1 << 17) * 1.0 * (1 << 17);	// too big for an int32, but exactly representable by a double
	id_ = OFFSET + Unique();
}

Cookie::Output_::~Output_()
{
	// discard any cookies held (probably an exception occurred before finalization)
	LOCK_OBJECTS;
	for (const auto& c : cookies_)
		TheStash().erase(c);
}

double Cookie::Output_::Finalize()
{
	LOCK_OBJECTS;
	TheBiscuits()[id_].cookies_.Swap(&cookies_);
	TheBiscuits()[id_].error_.clear();
	return id_;
}

double Cookie::Output_::Error(const char* what, const char* arg)
{
	String_ s(what);
	if (arg)
		s += " (at input '" + String_(arg) + "')";

	LOCK_OBJECTS;
	TheBiscuits()[id_].cookies_.clear();
	TheBiscuits()[id_].error_.Swap(&s);
	return id_;
}

void Cookie::Output_::AppendBase(const Handle_<Storable_>& h)
{
	cookies_.push_back(ToCookie(h));
}

// The vector/matrix versions of Output_::Append end up making an extra copy of their values
	// this could be avoided by making cookies non-const, and setting up a cookie to a mutable vector/matrix value before calling the internal analytics function
	// then passing the vector/matrix address to that internal function
	// this abitious optimization work is postponed
void Cookie::Output_::Append(const Vector_<>& v)
{
	cookies_.push_back(CopyToCookie(v));
}
void Cookie::Output_::Append(const Vector_<int>& v)
{
	cookies_.push_back(CopyToCookie(v));
}

// extract types for the internal interface
Cookie::Input_::~Input_()
{
	for (const auto& c : used_)
		StashDrop(c);
}

const Handle_<Storable_>& Cookie::Input_::HandleBase(int cookie, bool optional)
{
	struct Mine_ : boost::static_visitor<const Handle_<Storable_>&>
	{
		const Handle_<Storable_>& operator()(const Handle_<Storable_>& src) const { return src; }
		const Handle_<Storable_>& operator()(...) const { THROW("Input is not a handle"); }
	};

	used_.push_back(cookie);
	const Handle_<Storable_>& retval = boost::apply_visitor(Mine_(), *StashFind(cookie).val_);
	REQUIRE(retval || optional, "Input handle is empty");
	return retval;
}

const Vector_<>& Cookie::Input_::VectorDouble(int cookie)
{
	struct Mine_ : boost::static_visitor<const Vector_<>&>
	{
		const Vector_<>& operator()(const Handle_<Vector_<>>& src) const { return *src; }
		const Vector_<>& operator()(...) const { THROW("Input is not a vector-of-double"); }
	};

	used_.push_back(cookie);
	return boost::apply_visitor(Mine_(), *StashFind(cookie).val_);
}

const Vector_<int>& Cookie::Input_::VectorInt(int cookie)
{
	struct Mine_ : boost::static_visitor<const Vector_<int>&>
	{
		const Vector_<int>& operator()(const Handle_<Vector_<int>>& src) const { return *src; }
		const Vector_<int>& operator()(...) const { THROW("Input is not a vector-of-int"); }
	};

	used_.push_back(cookie);
	return boost::apply_visitor(Mine_(), *StashFind(cookie).val_);
}


//----------------------------------------------------------------------------
// the flat-C interface

int HandleCookie(void* ph)
{
	// ph must be a pointer-to-handle
	Handle_<Storable_>* mine = reinterpret_cast<Handle_<Storable_>*>(ph);
	return ToCookie(*mine);
}

int VectorDoubleCookie(double* vals, int size)
{
	Handle_<Vector_<>> mine = std::make_shared<const Vector_<>>(vals, vals + size);
	return ToCookie(mine);
}

int VectorIntCookie(int* vals, int size)
{
	Handle_<Vector_<int>> mine = std::make_shared<const Vector_<int>>(vals, vals + size);
	return ToCookie(mine);
}

int BiscuitCount(double id)
{
	LOCK_OBJECTS;
	auto pb = TheBiscuits().find(id);
	if (pb == TheBiscuits().end())
		return -1;	
	if (!pb->second.error_.empty())
		return -2;	
	return pb->second.cookies_.size();
}

int BiscuitCookie(double id, int which)
{
	if (which < 0)
		return which;	// can never be valid
	LOCK_OBJECTS;
	auto pb = TheBiscuits().find(id);
	if (pb == TheBiscuits().end())
		return -1;	// no such biscuit
	if (!pb->second.error_.empty())
		return -2;	// bad biscuit
	if (which >= pb->second.cookies_.size())
		return -3;	// invalid index-of-cookie
	int retval = pb->second.cookies_[which];
	if (retval + 1 == pb->second.cookies_.size())	// this is the last cookie
		TheBiscuits().erase(pb);
	return retval;
}

char* BiscuitError(double id)
{
	LOCK_OBJECTS;
	auto pb = TheBiscuits().find(id);
	if (pb == TheBiscuits().end())
		return nullptr;
	if (pb->second.error_.empty())
		return nullptr;	// no error
	char* retval = strdup(pb->second.error_.c_str());
	TheBiscuits().erase(pb);
	return retval;
}

// extract types for the foreign interface
void CookieRelease(int cookie)
{
	StashDrop(cookie);
}

void* CookieHandle(int cookie)
{
	static const Handle_<Storable_> EMPTY;
	struct Mine_ : boost::static_visitor<Handle_<Storable_>>
	{
		const Handle_<Storable_>& operator()(const Handle_<Storable_>& src) const { return src; }
		const Handle_<Storable_>& operator()(...) const { return EMPTY; }
	};

	const Stashable_& stored = StashFind(cookie);
	const Handle_<Storable_>& handle = boost::apply_visitor(Mine_(), *stored.val_);
	StashDrop(cookie);
	return handle.Empty() ? nullptr : new Handle_<Storable_>(handle);	// return pointer-to-handle; foreign language owns the memory now
}

int CookieVectorSize(int cookie)
{
	const Stashable_& stored = StashFind(cookie);
	return boost::apply_visitor(VectorSize_(), *stored.val_);	// don't drop the cookie from stash
}

const double* CookieVectorDoubleBase(int cookie)
{
	return boost::apply_visitor(VectorBase_<double>(), *StashFind(cookie).val_);
}

void CookieVectorDoubleCopyVals(int cookie, double* dst)
{
	struct Mine_ : boost::static_visitor<>
	{
		double* dst_;
		Mine_(double* dst) : dst_(dst) {}
		void operator()(const Handle_<Vector_<>>& src) const { std::copy(src->begin(), src->end(), dst_); }
		void operator()(...) const { }
	};

	const Stashable_& stored = StashFind(cookie);
	boost::apply_visitor(Mine_(dst), *stored.val_);
	StashDrop(cookie);
}

const int* CookieVectorIntBase(int cookie)
{
	return boost::apply_visitor(VectorBase_<int>(), *StashFind(cookie).val_);
}

void CookieVectorIntCopyVals(int cookie, int* dst)
{
	struct Mine_ : boost::static_visitor<>
	{
		int* dst_;
		Mine_(int* dst) : dst_(dst) {}
		void operator()(const Handle_<Vector_<int>>& src) const { std::copy(src->begin(), src->end(), dst_); }
		void operator()(...) const { }
	};

	const Stashable_& stored = StashFind(cookie);
	boost::apply_visitor(Mine_(dst), *stored.val_);
	StashDrop(cookie);
}


