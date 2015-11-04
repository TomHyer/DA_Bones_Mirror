
// public interface for testing
	// very simple functions to test FFI
#include "__Platform.h"
#include <cmath>

namespace
{
/*IF--------------------------------------------------------------------------
public Lorentz_Add
	Computes tanh(arctanh X + B * arctanh Y)
&inputs
X is number
	&fabs($) <= 1.0\$ must be in [-1, 1]
	as above
Y is number
	&fabs($) < 1.0\$ must be in (-1, 1)
	as above
&optional
B is number (1.0)
	&$ >= 0.0
	applies to Y only
&outputs
Z is number
	Will also be in (-1, 1) (or at the boundary, if X is)
-IF-------------------------------------------------------------------------*/

	void Lorentz_Add
		(double x,
		 double y,
		 double b,
		 double* z)
	{
		if (fabs(x) == 1.0)
			*z = x;
		else if (b == 1.0)
			*z = (x + y) / (1.0 + x * y);
		else
		{
			const double aZ = std::atanh(x) + b * std::atanh(y);
			*z = std::tanh(aZ);
		}
	}
}

#include "MG_Lorentz_Add_public.inc"

