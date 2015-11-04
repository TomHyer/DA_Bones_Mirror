
// valuation method choice and parameters of valuation process

#pragma once

#include "Strings.h"

/*IF--------------------------------------------------------------------------
enumeration ValuationMethod
	Top-level description of method
switchable
default CLOSED_FORM CF
alternative MONTE_CARLO MC
alternative PDE
-IF-------------------------------------------------------------------------*/
#include "MG_ValuationMethod_enum.h"

/*IF--------------------------------------------------------------------------
settings ValuationParameters
	Instructions how to carry out a valuation
&members
method is enum ValuationMethod default .
nPaths is integer default 5000
	Number of Monte Carlo simulations
-IF-------------------------------------------------------------------------*/
#include "MG_ValuationParameters_object.h"

