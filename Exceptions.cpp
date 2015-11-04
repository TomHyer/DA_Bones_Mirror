
#include "Platform.h"
#include "Exceptions.h"
//#include <boost/thread/tss.hpp>	// see comments on TheStack(), below
#include "Strict.h"

#include "Algorithms.h"
#include "DateTime.h"

using namespace Exception;

XStackInfo_::XStackInfo_(const char* name, const int& val)
:
name_(name), value_(&val), type_(Type_::INT)
{  }

XStackInfo_::XStackInfo_(const char* name, const double& val)
:
name_(name), value_(&val), type_(Type_::DBL)
{  }

XStackInfo_::XStackInfo_(const char* name, const String_& val)
:
name_(name), value_(&val), type_(Type_::STR)
{  }

XStackInfo_::XStackInfo_(const char* name, const char* val)
:
name_(name), value_(val), type_(Type_::CSTR)	// capture as char*, not char**
{  }

XStackInfo_::XStackInfo_(const char* name, const Date_& val)
:
name_(name), value_(&val), type_(Type_::DATE)
{  }

XStackInfo_::XStackInfo_(const char* name, const DateTime_& val)
:
name_(name), value_(&val), type_(Type_::DATETIME)
{  }

// also a NOTE constructor with just a message
XStackInfo_::XStackInfo_(const char* msg)
:
name_(msg), value_(nullptr), type_(Type_::VOID)
{	}

std::string XStackInfo_::Message() const
{
	static const std::string EQUALS(" = ");
	switch (type_)
	{
	case Type_::INT:
		return name_ + EQUALS + std::to_string(*(reinterpret_cast<const int*>(value_)));
	case Type_::DBL:
		return name_ + EQUALS + std::to_string(*(reinterpret_cast<const double*>(value_)));
	case Type_::CSTR:
		return name_ + EQUALS + std::string(reinterpret_cast<const char*>(value_));
	case Type_::STR:
		return name_ + EQUALS + reinterpret_cast<const String_*>(value_)->c_str();
	case Type_::DATE:
		return name_ + EQUALS + Date::ToString(*reinterpret_cast<const Date_*>(value_)).c_str();
	case Type_::DATETIME:
		return name_ + EQUALS + DateTime::ToString(*reinterpret_cast<const DateTime_*>(value_)).c_str();
	case Type_::VOID:
		return std::string(name_);
	}
	assert(!"Unrecognized StackInfo type");
	return std::string();
}

namespace
{
	/* more appropriate implementation for production
	Vector_<XStackInfo_>& TheStack()
	{
		static boost::thread_specific_ptr<Vector_<XStackInfo_>> INSTANCE;
		if (!INSTANCE.get())	// get is thread-specific
			INSTANCE.reset(new Vector_<XStackInfo_>);
		return *INSTANCE;	// so is operator*
	}
	*/

	/* less-efficient implementation, used here to avoid boost link dependency */
	Vector_<XStackInfo_>* XTheStack(bool free_if_empty = false)
	{
		__declspec(thread) static Vector_<XStackInfo_>* INSTANCE = nullptr;
		if (!INSTANCE)
			INSTANCE = new Vector_<XStackInfo_>;
		else if (free_if_empty && INSTANCE->empty())
		{
			delete INSTANCE;
			return INSTANCE = nullptr;
		}
		return INSTANCE;
	}
	Vector_<XStackInfo_>& TheStack() { return *XTheStack(); }

	std::string MsgWithStack(const char* msg)
	{
		std::string retval(msg);
		for (const auto& si : TheStack())
			retval += "; " + si.Message();
		return retval;
	}
}	// leave local

void Exception::PushStack(const XStackInfo_& info)
{
	TheStack().push_back(info);
}
void Exception::PopStack() 
{ 
	if (!TheStack().empty())
		TheStack().pop_back(); 

	// the following statement cleans up the stack pointer when it becomes empty
	// this prevents a memory leak (though not as reliably as a smart pointer implementation)
		// of course there is a runtime cost to the extra delete/new cycle
	// erase this line for production implementation using thread_specific_ptr
	(void) XTheStack(true);
}

Exception_::Exception_(const char* msg)
:
std::runtime_error(MsgWithStack(msg))
{	}

