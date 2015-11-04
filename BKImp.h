
// under-the-hood functions for Black-Karasinski model

#pragma once

#include "Storable.h"

namespace BK
{
	class Mapping_ : public Storable_
	{
	public:
		double rBar_;
		virtual double R(double S) const = 0;
		virtual double dRdS(double S) const = 0;
	};
}
