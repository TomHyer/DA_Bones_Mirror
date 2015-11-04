
// generator of sequences of pseudo-random numbers

#pragma once

class Random_
{
public:
	virtual ~Random_();
	virtual double NextUniform() = 0;
	virtual void FillUniform(Vector_<>* deviates) = 0;
	virtual void FillNormal(Vector_<>* deviates) = 0;
	virtual Random_* Branch(int i_child = 0) const = 0;
};

namespace Random
{
	Random_* New(int seed);
}

