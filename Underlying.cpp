
#include "Platform.h"
#include "Underlying.h"
#include "Strict.h"

namespace
{
   template<class T_> void MergeMax
      (T_* base,
      const T_& other)
   {
	   for (const auto& kv : other)
      {
         if (base->count(kv.first))
            (*base)[kv.first] = Max((*base)[kv.first], kv.second);
         else
            (*base)[kv.first] = kv.second;
      }
   }
}
Underlying_& Underlying_::operator+=(const Underlying_& other)
{
   MergeMax(&payCcys_, other.payCcys_);
   MergeMax(&indices_, other.indices_);
   MergeMax(&credits_, other.credits_);
   return *this;
}

Underlying_& Underlying_::Include(const Ccy_& ccy, const Date_& pay_date)
{
	if (!payCcys_.count(ccy))
		payCcys_[ccy] = pay_date;
	else
		payCcys_[ccy] = Max(payCcys_[ccy], pay_date);
	return *this;
}
Underlying_& Underlying_::Include(const IndexKey_& index, const DateTime_& fix_date)
{
	if (!indices_.count(index))
		indices_[index] = fix_date;
	else
		indices_[index] = Max(indices_[index], fix_date);
	return *this;
}
Underlying_& Underlying_::Include(const String_& credit, const Date_& pay_date)
{
	if (!credits_.count(credit))
		credits_[credit] = pay_date;
	else
		credits_[credit] = Max(credits_[credit], pay_date);
	return *this;
}
