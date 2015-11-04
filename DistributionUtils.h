
// helper functions for working with distributions polymorphically

#pragma once

#include "Distribution.h"

namespace Distribution
{
	double BlackIV
		(const Distribution_& model, 
		 double strike, 
		 double guess, 
		 int n_steps);
}