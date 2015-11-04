
// support models in their implementation of interest-rate slides

#pragma once

class YieldCurve_;
class Slide_;

enum class SlideEffect_ : char
{
   UNRECOGNIZED,
   ABSENT,
   NO_OP,
   EFFECTIVE
};


namespace SlideIr
{
	SlideEffect_ Apply
		(Vector_<Handle_<YieldCurve_> >* curves,
		 const Slide_& slide);
}
