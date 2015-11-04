
#include "Platform.h"
#include "IndexFx.h"
#include "Strict.h"

double Index::Fx_::Fixing(_ENV, const DateTime_& time) const
{
   const double test = PastFixing(_env, XName(false), time, true);
   return test > -DA::INFINITY
         ? test
         : 1.0 / PastFixing(_env, XName(true), time);
}

String_ Index::Fx_::XName(bool invert) const
{
   static const String_ SEP("/");
   return (invert ? dom_ : fgn_).String() + SEP + (invert ? fgn_ : dom_).String();
}

