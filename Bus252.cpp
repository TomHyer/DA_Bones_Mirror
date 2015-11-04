
#include "Platform.h"
// this source file has no associated header
#include "Strict.h"

#include "DayBasis.h"
#include "Holiday.h"

struct Bus252_DayBasis_ : DayBasis_::Extension_
{
   enum class Value_ : char
   {
      BRL_BUS_252,
      _Bus252_N_VALUES
   } val_;
   Bus252_DayBasis_(Value_ val) : val_(val) {}

   const char* String() const override;
   double operator()(const Date_& start_date, const Date_& end_date, const DayBasis::Context_* context) const override;
};

const char* Bus252_DayBasis_::String() const
{
   switch (val_)
   {
   case Value_::BRL_BUS_252:
      return "BRL_BUS_252";
   }
   assert(!"Invalid value in Bus252_DayBasis");
   return nullptr;
}

double Bus252_DayBasis_::operator()(const Date_& start_date, const Date_& end_date, const DayBasis::Context_*) const
{
   static const Holidays_ HOLS("BRL");
   return CountBusDays_(HOLS)(start_date, end_date) / 252.0;
}


RUN_AT_LOAD(
	Handle_<DayBasis_::Extension_> h(new Bus252_DayBasis_(Bus252_DayBasis_::Value_::BRL_BUS_252));
	DayBasis::RegisterExtension({ "BRL_BUS_252", "Brazil_BUS_252" }, h);
)


