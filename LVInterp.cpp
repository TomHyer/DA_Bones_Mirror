
#include "Platform.h"
#include "LVInterp.h"
#include "Strict.h"

#include "DateTime.h"
#include "Interp.h"

namespace
{
	class LVInterp_ : public LVSurface_
	{
		DateTime_ volStart_;
		Interp2::Of_<DateTime_> vals_;
		Interp1::Of_<DateTime_> loEdge_, loBeta_;
		Interp1::Of_<DateTime_> hiEdge_, hiBeta_;

		DateTime_ VolStartTime() const { return volStart_; }
		double LocalVol(const DateTime_& t, double s) const
		{
			const double lo = loEdge_(t);
			if (s < lo)
				return LocalVol(t, lo) * pow(s / lo, loBeta_(t) - 1.0);
			const double hi = hiEdge_(t);
			if (s > hi)
				return LocalVol(t, hi) * pow(s / hi, hiBeta_(t) - 1.0);
			return vals_(t, s);
		}
	};
}
