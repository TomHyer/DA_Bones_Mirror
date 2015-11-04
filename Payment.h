
// description of payments made by derivative contracts

#pragma once

#include "Strings.h"
#include "DateTime.h"
#include "AccrualPeriod.h"
#include "Optionals.h"
#include "Currency.h"

namespace Payment
{	
	// tags are held by trades to indicate a particular payment's circumstances
	class Tag_ : noncopyable
	{
	public:
		virtual ~Tag_();
	};
	const Handle_<Tag_>& Null();

	// make a distinction between things-with-value (payments) and mere amounts
	namespace Amount
	{
		class Tag_ : noncopyable
		{
		public:
			virtual ~Tag_();
		};
	}

	// separate tagging system for payment-on-default
	namespace Default
	{
		class Tag_ : noncopyable
		{
		public:
			virtual ~Tag_();
		};
	}

	
	// define when default must be, for a payment contingent on default
	struct DefaultPeriod_
	{
		Date_ start_;
		Date_ end_;
		bool recoverable_;	// scale amount by (1-R)
	};

	struct Conditions_
	{
		enum class Exercise_ : char
		{
			UNCONDITIONAL,
			ON_EXERCISE,
			ON_BARRIER_HIT,
			ON_CONTINUATION
		} exerciseCondition_;
		enum class Credit_ : char
		{
			RISKLESS,
			ON_SURVIVAL,
			ON_DEFAULT
		} creditCondition_;
		// if paid on default, we need still more info:
		DefaultPeriod_ defaultPeriod_;

		Conditions_();   // unconditional, riskless case
	};

	struct Info_
	{
		String_ description_;
		DateTime_ knownTime_;
		Conditions_ conditions_;
		boost::optional<AccrualPeriod_> period_;
		Info_(const String_& des = String_(),
			 const DateTime_& known = DateTime::Minimum(),
			 const Conditions_& cond = Conditions_(),
			 const AccrualPeriod_* accrual = nullptr);
	};
}

struct Payment_
{
	DateTime_ eventTime_;
	Ccy_ ccy_;
	Date_ date_;
	String_ stream_;
	Payment::Info_ tag_;
	Date_ commitDate_;	// will be compared to delivery
	Payment_();   // support Vector_<Payment_>
	Payment_(const Payment_& src);
	Payment_
		(const DateTime_& et, 
		 const Ccy_& ccy, 
		 const Date_& dt,
		 const String_& s, 
		 const Payment::Info_& tag,
		 const Date_& cd = Date::Minimum());
};

// value of a payment inside a numerical pricing run
class NodeValue_ : noncopyable
{
public:
	virtual ~NodeValue_();
	virtual void operator+=(double amount) = 0;
	// direct support for backward induction:
	// virtual double& operator*() = 0;
};

class NodeValues_ : noncopyable
{
public:
	virtual ~NodeValues_();
	virtual NodeValue_& operator[](const Payment::Tag_& tag) = 0;
	inline NodeValue_& operator[](const Handle_<Payment::Tag_>& tag)
	{
		return operator[](*tag);
	}

	virtual double& operator[](const Payment::Amount::Tag_& tag) = 0;
	inline double& operator[](const Handle_<Payment::Amount::Tag_>& tag)
	{
		return operator[](*tag);
	}
};

class NodeValuesDefault_ : noncopyable
{
public:
	virtual ~NodeValuesDefault_();
	virtual NodeValue_& operator()
		(const Payment::Default::Tag_& tag,
		 const Date_& commit_date) = 0;
	inline NodeValue_& operator()
		(const Handle_<Payment::Default::Tag_>& tag,
		 const Date_& commit_date)
	{
		return operator()(*tag, commit_date);
	}
};

