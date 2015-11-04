
// risk tasks, creating output reports

#pragma once

#include "Storable.h"
#include "Environment.h"

class Portfolio_;
class Model_;
class Report_;

class RiskTask_ : public Storable_
{
public:
	virtual Report_* Run
		(_ENV, const Portfolio_& portfolio,
		 const Model_& model)
	const = 0;
};
