
// Bermudan swaptions, supporting numerical pricing and query of European components

#pragma once

#include "Underlying.h"

class Swaption_;

class HasEuropeanComponents_ : public Underlying_::Parent_
{
public:
	virtual Vector_<Handle_<Swaption_>> EuropeanComponents() const = 0;
};
