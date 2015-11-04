
// basic description of a default event

#pragma once

#include "Date.h"
#include "Strings.h"
#include "Vectors.h"

struct CreditId_
{
	int val_;
	explicit CreditId_(int p) : val_(p) {}
};
inline bool operator<(const CreditId_& lhs,	const CreditId_& rhs) {	return lhs.val_ < rhs.val_; }

struct ObservedDefault_
{
	Date_ date_;
	CreditId_ referenceName_;
	double recovery_;
};

struct AssignCreditId_ : noncopyable
{
	Vector_<String_> names_;   // constructor sorts these
	AssignCreditId_(const Vector_<String_>& names);
	CreditId_ operator()(const String_& name);
};
