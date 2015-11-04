
// discount curves just compute discount factors

#pragma once

#include "YCComponent.h"

class Date_;

class DiscountCurve_ : public YCComponent_
{
public:
	DiscountCurve_(const String_& name);
	virtual double operator()(const Date_& from, const Date_& to) const = 0;
};

