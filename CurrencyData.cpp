
#include "Platform.h"
#include "CurrencyData.h"
#include "Strict.h"

#include "Exceptions.h"
#include "Facts.h"
#include "Holiday.h"
#include "PeriodLength.h"
#include "CouponRate.h"

namespace
{
   template<class T_> struct CcyDependent_
   {
      Handle_<T_> background_;
      std::map<Ccy_, Handle_<T_> > specific_;
   };

	template<class T_> class XFactWriterImp_ : public Ccy::Fact_<T_>::Writer_
	{
		CcyDependent_<T_>& data_;   // we do not own
	public:
		XFactWriterImp_(CcyDependent_<T_>& dat) : data_(dat) {}

		void SetDefault(const T_& val)
		{
			data_.background_.reset(new T_(val));
		}
		void operator()(const Ccy_& ccy, const T_& val)
		{
			data_.specific_[ccy].reset(new T_(val));
		}
	};

	template<class T_> class OneFactImp_ : public Ccy::Fact_<T_>
	{
		CcyDependent_<T_> vals_;
		typedef typename Ccy::Fact_<T_>::Writer_ writer_t;
		std::unique_ptr<writer_t> writer_;
	public:
		OneFactImp_()
		{
			writer_.reset(new XFactWriterImp_<T_>(vals_));
		}

		const T_& operator()(const Ccy_& ccy) const
		{         
         auto pc = vals_.specific_.find(ccy);
			if (pc != vals_.specific_.end())
				return *pc->second;
			REQUIRE(!vals_.background_.Empty(), "No default for '" + String_(ccy.String()) + "'");
			return *vals_.background_;
		}
		writer_t& XWrite() const { return *writer_; }
	};
}	// leave local


#define SINGLETON_FACT_ACCESSOR(type, func)			\
const Ccy::Fact_<type>& Ccy::Conventions::func()	\
{ RETURN_STATIC(const OneFactImp_<type>); }

SINGLETON_FACT_ACCESSOR(Holidays_, SwapPayHolidays)
SINGLETON_FACT_ACCESSOR(Holidays_, LiborFixHolidays)
SINGLETON_FACT_ACCESSOR(int, LiborFixDays)
SINGLETON_FACT_ACCESSOR(DayBasis_, LiborDayBasis)
SINGLETON_FACT_ACCESSOR(DayBasis_, SwapFixedDayBasis)
SINGLETON_FACT_ACCESSOR(PeriodLength_, SwapFixedPeriod)
SINGLETON_FACT_ACCESSOR(TradedRate_, SwapFloatIndex)


