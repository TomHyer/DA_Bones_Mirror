
// collateralization is a trade attribute, affects discount factors

#pragma once

class String_;

/*IF--------------------------------------------------------------------------
enumeration CollateralType
   Quantities for which discount curves are defined
switchable
alternative OIS
   Collateral appropriate for OIS or similar rate
alternative GC
   General gov't collateral
alternative NONE
-IF-------------------------------------------------------------------------*/

#include "MG_CollateralType_enum.h"

/*IF--------------------------------------------------------------------------
enumeration Clearer
   Identifies a clearinghouse
switchable
alternative CME
alternative LCH
method CollateralType_ Collateral() const;
-IF-------------------------------------------------------------------------*/

#include "MG_Clearer_enum.h"

