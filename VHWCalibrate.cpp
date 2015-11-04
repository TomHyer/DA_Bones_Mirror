
#include "Platform.h"
#include "VHWCalibrate.h"
#include "Strict.h"

#include "Exceptions.h"
#include "Rootfind.h"
#include "Semianalytic.h"
#include "Swaption.h"

Model_* VHW::MatchSwaptionHoLee
	(const String_& name,
	 const Handle_<YieldCurve_>& yc,
	 const Swaption_& swaption,
	 double value,
	 const DateTime_& vol_start)
{
	static const int MAX_ITERATIONS = 40;
	static const PositiveIncreasing_ ToVol(0.01);
	static const double TOL = 1.0e-8;
	Brent_ task(0.0, TOL);	// will be mapped to vol of 1%
	Converged_ check(TOL, TOL);
	std::unique_ptr<Model_> model;
	for (int ii = 0; ii < MAX_ITERATIONS; ++ii)
	{
		const double vol = ToVol(task.NextX());
		model.reset(VHW::NewHoLee(name, yc, vol_start, vol));
		const double price = Semianalytic::Value(nullptr, swaption, *model)[0].second;	// maybe eventually there will be a special-purpose routine for this
		if (check(task, price - value))
			return model.release();	// success
	}
	THROW("Exhausted iterations in Ho-Lee calibration");
}

