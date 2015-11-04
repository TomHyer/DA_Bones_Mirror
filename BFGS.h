/*
*      Limited memory BFGS (L-BFGS).
*
* Based on Okazaki's C port of Nocedal's Fortran library L-BFGS.
* This implementation is simplified from the above, in that it does not support Orthant-Wise or SSE optimization.
*
* The original FORTRAN source code is available at:
* http://www.ece.northwestern.edu/~nocedal/lbfgs.html
* The C source code of liblbfgs is available at:
* http://www.chokkan.org/software/liblbfgs/
*/

#include "Platform.h"
//#include "BFGS.h"
#include "Strict.h"

#include "Strings.h"
namespace BFGS
{
	class Func_;
}
struct BFGSControls_;

/*IF--------------------------------------------------------------------------
enumeration LinesearchMethod
	Line search algorithm supporting BFGS
switchable
default MORE_THUENTE
alternative ARMIJO
	The backtracking method finds the step length such that it satisfies the sufficient decrease
	(Armijo) condition, - f(x + a * d) <= f(x) + BFGSControls_::ftol * a * g(x)^T d, where
	x is the current point, d is the current search direction, and a is the step length.
alternative WOLFE
	Finds the step length such that it satisfies both the Armijo condition and the curvature
	condition, - g(x + a * d)^T d >= BFGSControls_::wolfe * g(x)^T d
alternative STRONG_WOLFE
	Finds the step length such that it satisfies both the Armijo condition and the following
	condition,  - |g(x + a * d)^T d| <= BFGSControls_::wolfe * |g(x)^T d|
method int LineSearch(Vector_<>* x, double* f, Vector_<>* g, const Vector_<>& s, double* stp, const Vector_<>& xp, const Vector_<>& gp, const BFGS::Func_& func, const BFGSControls_& controls) const;
-IF-------------------------------------------------------------------------*/

#include "MG_LinesearchMethod_Enum.h"

/*IF--------------------------------------------------------------------------
object BFGSControls
	L-BFGS optimization parameters
is_settings
&members
m is integer default 6
	The number of corrections to approximate the inverse hessian matrix.
	Large values will result in excessive computing time.
epsilon is number default 0.00001
	A minimization terminates when ||g|| < \ref epsilon * max(1, ||x||)
past is integer default 0
	This parameter determines the distance, in iterations, to compute the rate of decrease of the
	objective function. If zero, no delta-based convergence test is done.
delta is number default 0.00001
	A minimization terminates when f'/f < 1+delta, where f' is the objective value of past
	iterations ago and f is the current objective value.
max_iterations is integer default 0
	If nonzero, sets a maximum number of iterations
linesearch is enum LinesearchMethod
max_linesearch is integer default 40
	The maximum number for an iteration of the line search
min_step is number default 1e-20
	Minimum step of the line search routine.  Should only need changing if the problem is extremely
	badly scaled
max_step is number default 1e20
	Maximum step of the line search routine.  Should only need changing if the problem is extremely
	badly scaled
ftol is number default 0.0001
	A parameter to control the accuracy of the line search routine.
wolfe is number default 0.9
	A coefficient for the Wolfe condition.
gtol is number default 0.9
	A parameter to control the accuracy of the line search routine.  If function and gradient
	evaluations are fast, a smaller value (e.g. 0.1) may be used.
halt_on_stall is boolean default false
	If true, allow underflow of step
&conditions
m_ >= 3
epsilon_ > 0
past_ >= 0
delta_ >= 0.0
max_iterations_ >= 0
ftol_ > 0.0 && ftol_ < 0.5
wolfe_ > ftol_ && wolfe_ < 1.0
gtol_ > ftol_ && gtol_ < 1.0
-IF-------------------------------------------------------------------------*/

#include "MG_BFGSControls_Object.h"

namespace BFGS
{ 
	class Func_ : noncopyable
	{
	public:
		virtual ~Func_();

		// return function value and also populate gradient
		virtual double operator()
			(const Vector_<>& x,
			 Vector_<>* g,	// function must compute the gradient as well
			 double step)	// current step of the line search routine
		const = 0;

		// take state-of-search information, return true to halt
		// this function is a const member -- using it to update mutable internal data is legal but may jeopardize thread safety
		virtual bool CheckHalt
			(const Vector_<>& x,	// current location
			const Vector_<>& g,	// current gradient
			double fx,				// current objective function
			double xnorm,			// norm of variables
			double gnorm,			// norm of gradient
			double step,			// current line-search step
			int k,					// iteration count
			int ls)				// evaluations for this iteration
		const
		{
			return false;
		}
	};

	void Minimize
		(Vector_<>* x,	// on input, initial point; on output, minimizing point
		 const Func_& func,
		 const BFGSControls_& controls,
		 double* fx = 0);		// for optional output of f(x) at minimum
}

