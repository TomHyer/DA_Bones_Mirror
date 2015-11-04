
// public interface to date functions

#include "__Platform.h"
#include "Strict.h"

#include "DateIncrement.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Date_Increment
   Applies a text increment to a date
&inputs
from is date
   The starting point
step is string
   The text describing the increment
&optional
backward is boolean (false)
   By default, the step is forward
&outputs
to is date
   The result of incrementing the input date
-IF-------------------------------------------------------------------------*/

   void Date_Increment
      (const Date_& from,
       const String_& step,
       bool backward,
       Date_* to)
   {
      auto increment = Date::ParseIncrement(step);
      *to = backward ? increment->BackFrom(from) : increment->FwdFrom(from);
   }

/*IF--------------------------------------------------------------------------
public DateTime_Text
	Converts a datetime to a string representation (mostly for testing)
&inputs
t is time
	To be represented as string
&outputs
s is string
secs is integer
	Number of seconds in the unix epoch
-IF---------------------------------------------------------------------------*/

   void DateTime_Text(const DateTime_& t, String_* s, int* secs)
   {
	   *s = DateTime::ToString(t);
	   *secs = static_cast<int>(DateTime::MSec(t) / 1000);
   }
}  // leave local

#include "MG_Date_Increment_public.inc"
#include "MG_DateTime_Text_public.inc"


