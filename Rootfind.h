
// rootfinders encapsulating the state of the search

#pragma once

class Rootfinder_
{
public:
	virtual ~Rootfinder_();
	virtual double NextX() = 0;
	virtual void PutY(double y) = 0;
	virtual double BracketWidth() const = 0;
};

// convergence-checking utility
struct Converged_
{
	double xtol_, ftol_;
	Converged_(double x, double f) : xtol_(x), ftol_(f) {}
	bool operator()(Rootfinder_& t, double e) const
	{
		t.PutY(e);
		return fabs(e) < ftol_ || t.BracketWidth() < xtol_;
	}
};

// helper for mapping reals to positive reals
struct PositiveIncreasing_
{
	double scale_;
	PositiveIncreasing_(double s) : scale_(s) {}
	double operator()(double x) const { return scale_ * (x > 0.0 ? 1.0 + x : exp(x)); }
	double Inverse(double s) const { return s > scale_ ? s / scale_ - 1.0 : log(s / scale_); }
};

// specific search engine:  Brent's method
class BracketedBrent_ : public Rootfinder_
{
	pair<double, double> a_, b_, c_;
	const double tol_;
	bool bisect_;
	double d_;

	friend class Brent_;
	BracketedBrent_(double tol) : tol_(tol) {}	// uninitialized state
	void Initialize
		(const pair<double, double>& low,
		 const pair<double, double>& high);
public:
	BracketedBrent_
		(const pair<double, double>& low,
		 const pair<double, double>& high,
		 double tolerance);

	double NextX() override;
	void PutY(double y) override;
	double BracketWidth() const override { return fabs(a_.first - b_.first); }
};

class Brent_ : public Rootfinder_
{
	// state for a non-bracketed rootfinder
	enum class Phase_ { INITIALIZE, HUNT, BRACKETED } phase_;
	bool increasing_;   // guide direction of hunt
	double stepSize_, trialX_;
	pair<double, double> knownPoint_;
	// state after bracketing
	BracketedBrent_ engine_;
public:
	Brent_
		(double guess, double tolerance = DA::EPSILON,
		double step_size = 0.0);
	double NextX() override;
	void PutY(double y) override;
	double BracketWidth() const override;
};

