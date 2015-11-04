
#include "Platform.h"
#include "Rootfind.h"
#include "Strict.h"

Rootfinder_::~Rootfinder_()
{	}

BracketedBrent_::BracketedBrent_
	(const pair<double, double>& low,
	const pair<double, double>& high,
	double tolerance)
:
tol_(tolerance)
{
	Initialize(low, high);
}

void BracketedBrent_::Initialize
	(const pair<double, double>& low,
	const pair<double, double>& high)
{
	a_ = low;
	b_ = high;
	assert(a_.second * b_.second <= 0.0);
	if (fabs(a_.second) < fabs(b_.second))
		swap(a_, b_);	// b is best guess
	c_ = a_;
	bisect_ = true;
}

double BracketedBrent_::NextX()
{
	double s;
	if (c_.second != a_.second && c_.second != b_.second)
	{	// inverse quadratic interpolation
		s = a_.first * b_.second * c_.second / ((a_.second - b_.second) * (a_.second - c_.second))
			+ b_.first * a_.second * c_.second / ((b_.second - a_.second) * (b_.second - c_.second))
			+ c_.first * a_.second * b_.second / ((c_.second - a_.second) * (c_.second - b_.second));
	}
	else
	{	// secant interpolation
		s = (a_.first * b_.second - b_.first * a_.second) / (b_.second - a_.second);
	}
	double cDist = fabs(c_.first - (bisect_ ? b_.first : d_));	// d_ won't be used before it is set, because bisect_ is set at start
	bisect_ = (s - b_.first) * (s - 0.75 * a_.first - 0.25 * b_.first) >= 0.0
		|| fabs(s - b_.first) > 0.5 * cDist
		|| cDist < tol_;
	if (bisect_)
		s = 0.5 * (a_.first + b_.first);
	// done with d_; use it to hold s
	d_ = s;
	return d_;
}

void BracketedBrent_::PutY(double f_s)
{
	const double s = d_;	// fetch it back from where we stored it
	d_ = c_.first;
	c_ = b_;
	(f_s * a_.second > 0 ? a_ : b_) = make_pair(s, f_s);
	if (fabs(a_.second) < fabs(b_.second))
		swap(a_, b_);	// b is best guess
}

// preliminary hunt phase, followed by BracketedBrent

Brent_::Brent_
	(double guess,
	double tolerance,
	double step_size)
:
phase_(Phase_::INITIALIZE),
increasing_(true),	// wild guess
stepSize_(step_size > 0.0 ? step_size : 0.1 * Max(0.01, fabs(guess))),
trialX_(guess),
knownPoint_(DA::INFINITY, DA::INFINITY),
engine_(tolerance)	// store for when it is needed
{	}

double Brent_::NextX()
{
	return phase_ == Phase_::BRACKETED
		? engine_.NextX()
		: trialX_;
}

void Brent_::PutY(double y)
{
	static const double EXPANSION = exp(1.0);
	switch (phase_)
	{
	case Phase_::INITIALIZE:
		knownPoint_ = make_pair(trialX_, y);
		trialX_ += stepSize_ * (increasing_ ? 1 : -1) * (y < 0.0 ? 1 : -1);
		phase_ = Phase_::HUNT;
		return;
	case Phase_::BRACKETED:
		engine_.PutY(y);
		return;
	case Phase_::HUNT:
		if (y * knownPoint_.second <= 0.0)
		{
			engine_.Initialize(knownPoint_, make_pair(trialX_, y));
			phase_ = Phase_::BRACKETED;
			return;
		}
		// need to widen the hunt
		if (fabs(y) > fabs(knownPoint_.second))
			increasing_ = !increasing_;	// reverse direction, we are going away from the root
		else
			knownPoint_ = make_pair(trialX_, y);	// take the new point, we are going toward the root
		stepSize_ *= EXPANSION;
		trialX_ = knownPoint_.first + (increasing_ ? 1 : -1) * (knownPoint_.second < 0.0 ? 1 : -1) * stepSize_;
	}
}

double Brent_::BracketWidth() const
{
	return phase_ == Phase_::BRACKETED
		? engine_.BracketWidth()
		: DA::INFINITY;
}

