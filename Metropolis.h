
// Implementation of Metropolis-style simulated annealing algorithms

#include "Vectors.h"
#include "Random.h"

namespace Metropolis
{
	struct State_
	{
		pair<Vector_<>, double> best_;
		int nToGo_;
		double tau_, tauDecay_;
		Random_& rands_;

		State_(int n_iterations, const Vector_<>& x_init, double f_init, double t_init, double half_life, Random_& rand_src) : best_(x_init, f_init), nToGo_(n_iterations), tau_(t_init), tauDecay_(pow(0.5, 1.0 / half_life)), rands_(rand_src) {}

		bool Update(const Vector_<>& x_test, double f_test)
		{
			if (f_test > best_.second - tau_ * log(rands_.NextUniform()))
				return false;
			best_ = { x_test, f_test };
			return true;
		}
		bool Complete() { tau_ *= tauDecay_;  return nToGo_-- <= 0; }
	};
}

/* example usage:

while (!state.CheckHalt())
{
	xTest = step(state.best_.first, rand);
	(void)state.Update(xTest, eval(xTest))
	step.Smaller();
}
*/

