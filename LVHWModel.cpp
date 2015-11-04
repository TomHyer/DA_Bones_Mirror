
#include "Platform.h"
#include "LVHWModel.h"
#include "Strict.h"

#include "Vectors.h"
#include "Algorithms.h"

#ifdef debug
static const Vector_<> STATE_PADDING(4, DA::NAN);
#else
static const Vector_<> STATE_PADDING;
#endif

/*
Vector_<pair<double, double> > LVHWAccumulator_::Envelope
	(const DateTime_& t,
	 double num_sigma)
{
	static const Vector_<pair<double, double> > PADDING = Zip(STATE_PADDING, STATE_PADDING);

	Vector_<pair<double, double> > retval = ir_->Envelope(t, num_sigma);
	for (const auto& e : eq_)
	{
		Append(&retval, PADDING);
		Append(&retval, e->Envelope(t, num_sigma));
	}
	return retval;
}

*/