
#include "Platform.h"
#include "BackwardInduction.h"
#include "Strict.h"

BackwardInduction::Action_::Action_
   (const String_& stream,
    const DateTime_& event,
    const Date_& delivery)
: stream_(stream), eventTime_(event), deliveryDate_(delivery)
{  }

