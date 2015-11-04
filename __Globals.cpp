
#include "__Platform.h"
#include "Globals.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Global_SetAccountingDate
   Set a new value for the global accounting date
&inputs
date is date
   The accounting date to use hereafter
&outputs
new_date is date
   Confirmation of the new date value
old_date is date
   The previous value of the global accounting date
-IF-------------------------------------------------------------------------*/

   void Global_SetAccountingDate
      (const Date_& date,
       Date_* new_date,
       Date_* old_date)
   {
      ENV_SEED_TYPE(Global::Dates_);   // grab the access we need
      *old_date = Environment::Find<Global::Dates_>(_env)->AccountingDate();
      XGLOBAL::SetAccountingDate(date);
      *new_date = Environment::Find<Global::Dates_>(_env)->AccountingDate();
   }
}  // leave local

#include "MG_Global_SetAccountingDate_public.inc"

