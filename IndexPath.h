
// whole-path analysis of indices (e.g. for range accruals)

#pragma once

#include <map>
#include "DateTime.h"

class IndexPath_ : noncopyable
{
public:
	virtual ~IndexPath_();

	virtual double Expectation
		(const DateTime_& fixing_time,
		 const pair<double, double>& collar)
	const = 0;

	virtual double FixInRangeProb
		(const DateTime_& fixing_time,
		 const pair<double, double>& range,
		 double ramp_width = 0.0)
	const = 0;

	// Probability of staying strictly inside a range
	virtual double AllInRangeProb
		(const DateTime_& from,
		 const DateTime_& to,
		 const pair<double, double>& range,
		 double monitoring_interval,
		 double ramp_sigma = 0.0)
	const = 0;

	virtual double Extremum
		(bool maximum,
		 const DateTime_& from,
		 const DateTime_& to,
		 double monitoring_interval,
		 const pair<double, double>& collar)
	const;   // default calls AllInRangeProb
};

// special case where everything is known
struct IndexPathHistorical_ : IndexPath_
{
	std::map<DateTime_, double> fixings_;
	double Expectation
		(const DateTime_& t,
		 const pair<double, double>& lh)
	const override;
};

