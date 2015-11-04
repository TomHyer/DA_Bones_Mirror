
#include "Platform.h"
#include "BuySell.h"
#include "Strict.h"

#include "Exceptions.h"

#include "MG_BuySell_enum.inc"

int BuySell_::Sign() const
{
	return val_ == Value_::BUY ? 1 : -1;
}

