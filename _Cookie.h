
// 'Cookie' interface for foreign-function calls
// the foreign language registers the real inputs, and receives a cookie for each
// then passes the cookies to an interface function, and receives a "biscuit" (a nested output cookie) in return

#pragma once

#include "Storable.h"
#include "Vectors.h"

// flat-C interface functions called by foreign language
extern "C"
{
	__declspec(dllexport) void HandleDelete(void* ph);
	// convert function inputs to cookies
	__declspec(dllexport) int HandleCookie(void* ph);

	__declspec(dllexport) int VectorDoubleCookie(double* vals, int size);
	__declspec(dllexport) int VectorIntCookie(int* vals, int size);

	// convert the return biscuit into a series of cookies
	__declspec(dllexport) int BiscuitCount(double biscuit);
	__declspec(dllexport) int BiscuitCookie(double biscuit, int which);		// get cookies in order -- getting the last one discards the biscuit
	// also handle errors here
	__declspec(dllexport) char* BiscuitError(double biscuit);	// returns nullptr if good; returns strdup of the error, and discards the biscuit, if bad

	// then extract the real output from each cookie
	// unless otherwise commented, each of these functions erases the cookie it calls
	// the foreign language wrapper is responsible for ensuring all cookies are erased
	__declspec(dllexport) void CookieRelease(int cookie);	// manually destroy the cookie

	__declspec(dllexport) void* CookieHandle(int cookie);

	__declspec(dllexport) int CookieVectorSize(int cookie);	// does not destroy the cookie, because we need it to get vals

	__declspec(dllexport) const double* CookieVectorDoubleBase(int cookie);	// does not destroy the cookie, so values stay in memory
	__declspec(dllexport) void CookieVectorDoubleCopyVals(int cookie, double* dst);

	__declspec(dllexport) const int* CookieVectorIntBase(int cookie);	// does not destroy the cookie, so values stay in memory
	__declspec(dllexport) void CookieVectorIntCopyVals(int cookie, int* dst);
}

// helper functions for our wrapper interfaces
namespace Cookie
{
	// cookies can only be used once -- set up so that fetching a reference simultaneously marks the cookie as used
	struct Input_
	{
		Vector_<int> used_;
		~Input_();	// discards used cookies

		const Handle_<Storable_>& HandleBase(int cookie, bool optional);
		template<class T_> Handle_<T_> Handle(int cookie, bool optional = false)
		{
			Handle_<Storable_> temp = HandleBase(cookie, optional);
			Handle_<T_> retval = handle_cast<T_>(temp);
			REQUIRE(retval || !temp, "Input handle has wrong type");
			return retval;
		}

		const Vector_<>& VectorDouble(int cookie);
		const Vector_<int>& VectorInt(int cookie);
	};

	class Output_	// the biscuit
	{
		double id_;	// returned to foreign language, and used as cache key to support Biscuit... functions
		Vector_<int> cookies_;
	public:
		Output_();	// provides a unique ID
		~Output_();	//	discards any non-finalized cookies
		double Finalize();	// adds contents to the cache -- idiomatic use is "return ob.Finalize();"
		double Error(const char* what, const char* arg);	// discards contents and adds error to the cache -- idiomatic "return ob.Error();"

		void AppendBase(const Handle_<Storable_>& h);
		template<class T_> void Append(const Handle_<T_>& h) { AppendBase(handle_cast<Storable_>(h)); }

		void Append(const Vector_<>& v);
		void Append(const Vector_<int>& v);
	};
}
