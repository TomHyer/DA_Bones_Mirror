
// instruments used to calibrate yield curves

#pragma once

class YieldCurve_;
class String_;
class Date_;

class YcInstrument_ : noncopyable
{
public:
	virtual ~YcInstrument_();
	virtual String_ Name() const = 0;
	virtual pair<Date_, Date_> TimeSpan() const = 0;

	struct Rate_ : noncopyable
	{
		virtual ~Rate_();
		virtual double operator()
			(const YieldCurve_& yc)
		const = 0;
	};
	virtual Handle_<Rate_> Precompute
		(const Handle_<YcInstrument_>& self,
       const Handle_<YieldCurve_>& funding_yc)
	const = 0;
};

