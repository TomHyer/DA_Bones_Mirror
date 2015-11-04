
// exception class, with enhanced information attached upon constructio
// not suitable for low-level exceptions, since its constructor may allocate memory

#pragma once

#include <exception>
#include "Strings.h"
#include "Vectors.h"
class Date_;
class DateTime_;

class Exception_ : public std::runtime_error
{
public:
	Exception_(const char* msg);
	Exception_(const std::string& msg) : Exception_(msg.c_str()) {}
	Exception_(const String_& msg) : Exception_(msg.c_str()) {}
};
#define THROW(msg) throw Exception_(msg);
#define REQUIRE(cond, msg) if (cond); else THROW(msg);

// support for NOTE and NOTICE
namespace Exception
{
	class XStackInfo_
	{
		const char* name_;
		const void* value_;
		enum class Type_ { INT, DBL, CSTR, STR, DATE, DATETIME, VOID } type_;
		template<class T_> XStackInfo_(const char*, T_) {}

	public:
		XStackInfo_(const char* name, const int& val);
		XStackInfo_(const char* name, const double& val);
		XStackInfo_(const char* name, const char* val);
		XStackInfo_(const char* name, const Date_& val);
		XStackInfo_(const char* name, const DateTime_& val);
		XStackInfo_(const char* name, const String_& val);
		XStackInfo_(const char* msg);	// for VOID type
		std::string Message() const;	// not String_ because we need to interact with runtime_error
	};

	void PushStack(const XStackInfo_& info);
	void PopStack();

	struct StackRegister_
	{
		~StackRegister_() { Exception::PopStack(); }
		// constructor MUST push something on the stack
		template<class T_> StackRegister_(const char* name, const T_& val)
		{
			PushStack(XStackInfo_(name, val));
		}
		StackRegister_(const char* msg)
		{
			PushStack(XStackInfo_(msg));
		}
	};
}

#define XXNOTICE(u, n, v) Exception::StackRegister_ __xsr##u(n, v)
#define XNOTICE(u, n, v) XXNOTICE(u, n, v)
#define NOTICE2(n, v) XNOTICE(__COUNTER__, n, v)
#define NOTICE(x) NOTICE2(#x, x)

#define XXNOTE(u, m) Exception::StackRegister_ __xsr##u(m)
#define XNOTE(u, m) XXNOTE(u, m)
#define NOTE(msg) XNOTE(__COUNTER__, msg)
