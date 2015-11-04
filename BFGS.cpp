
#include "Platform.h"
#include "BFGS.h"
#include "Strict.h"

#include "Functionals.h"
#include "Numerics.h"
#include "Algorithms.h"
#include "Vectors.h"
#include "Exceptions.h"
#include "CellUtils.h"

#include "MG_LinesearchMethod_Enum.inc"
#include "MG_BFGSControls_Object.inc"

BFGS::Func_::~Func_()
{	}

/*
The L-BFGS algorithm is described in:
- Jorge Nocedal.
Updating Quasi-Newton Matrices with Limited Storage.
<i>Mathematics of Computation</i>, Vol. 35, No. 151, pp. 773--782, 1980.
- Dong C. Liu and Jorge Nocedal.
On the limited memory BFGS method for large scale optimization.
<i>Mathematical Programming</i> B, Vol. 45, No. 3, pp. 503-528, 1989.

The line search algorithms used in this implementation are described in:
- John E. Dennis and Robert B. Schnabel.
<i>Numerical Methods for Unconstrained Optimization and Nonlinear
Equations</i>, Englewood Cliffs, 1983.
- Jorge J. More and David J. Thuente.
Line search algorithm with guaranteed sufficient decrease.
<i>ACM Transactions on Mathematical Software (TOMS)</i>, Vol. 20, No. 3,
pp. 286-307, 1994.
*/

double Max(double a, double b, double c) { return Max(Max(a, b), c); }

struct IterationData_ 
{
	double alpha;
	Vector_<> s;
	Vector_<> y;
	double yDotS;  

	IterationData_(int n = 0) : alpha(0.0), s(n, 0.0), y(n, 0.0), yDotS(0.0) {}
};


/* Forward function declarations. */

// unwind-protect in case linesearch throws an exception
template<class T_> struct SetInScope_
{
	T_ save_;
	T_* dst_;
	SetInScope_(T_* parent) : save_(*parent), dst_(parent) {}
	SetInScope_(T_* parent, T_ temp) : save_(*parent), dst_(parent) { *parent = temp; }
	~SetInScope_() { ASSIGN(dst_, save_); }
	void release() { dst_ = nullptr; }
};

static void update_trial_interval(
	double* x,
	double *fx,
	double* dx,
	double *y,
	double *fy,
	double *dy,
	double *t,
	double *ft,
	double *dt,
	double tmin,
	double tmax,
	int *brackt);

void BFGS::Minimize
	(Vector_<>* x,	// on input, initial point; on output, minimizing point
	 const BFGS::Func_& func,
	 const BFGSControls_& param,
	 double* fx_final)		// for optional output of f(x) at minimum
{
	int i, j, k, ls, end, bound;
	double step;

	/* Constant parameters and their default values. */
	const int n = x->size();
	const int& m = param.m_;

	Vector_<> xp(n), g(n), gp(n), pg(n), d, w(n);
	Vector_<IterationData_> lm(n, IterationData_(n));
	double yDotS, yy;
	double xnorm, gnorm;
	double rate = 0.;
	Vector_<double> pf(param.past_, 0.0);
	// if user requested fx_final, keep it up to date
	double fxown;
	double& fx = fx_final ? *fx_final : fxown;

	fx = func(*x, &g, 0.0);

	// Store the initial value of the objective function. 
	if (param.past_)
		pf[0] = fx;

	// Compute the direction; we assume the initial hessian matrix H_0 as the identity matrix.
	d = Apply(std::negate<double>(), g);

	//	Make sure that the initial variables are not a minimizer.
	xnorm = Max(1.0, sqrt(InnerProduct(*x, *x)));
	gnorm = sqrt(InnerProduct(g, g));
	if (gnorm / xnorm <= param.epsilon_)
		return;

	// Compute the initial step:
	step = 1.0 / sqrt(InnerProduct(d, d));

	k = 1;
	end = 0;
	for (;;) 
	{
		// Store the current position and gradient vectors. 
		xp = *x;
		gp = g;

		{	// braces for scoping
			SetInScope_<Vector_<>> saveX(x);

			// Search for an optimal step.
			ls = param.linesearch_.LineSearch(x, &fx, &g, d, &step, xp, gp, func, param);
			assert(ls >= 0);	// should throw otherwise
			saveX.release();
		}

		/* Compute x and g norms. */
		xnorm = sqrt(InnerProduct(*x, *x));
		gnorm = sqrt(InnerProduct(g, g));

		if (func.CheckHalt(*x, g, fx, xnorm, gnorm, step, k, ls))
			return;

		// Convergence test.
		xnorm = Max(1.0, xnorm);
		if (gnorm / xnorm <= param.epsilon_)
			return;	// success

		// Test for stopping criterion based on improvement from past
		if (param.past_)
		{ 
			if (k >= param.past_)	// Compute the relative improvement from the past.
				if (pf[k % param.past_] - fx < param.delta_)
					return;
			/* Store the current value of the objective function. */
			pf[k % param.past_] = fx;
		}

		REQUIRE(!param.max_iterations_ || k < param.max_iterations_, "Exhausted iterations in BFGS");

		// s_{k+1} = x_{k+1} - x_{k} = \step * d_{k}.
		// y_{k+1} = g_{k+1} - g_{k}.
		auto& it = lm[end];
		Transform(*x, xp, std::minus<double>(), &it.s);
		Transform(g, gp, std::minus<double>(), &it.y);

		// yDotS = y \cdot s = 1 / \rho.
		// yy = y \cdot y, used for scaling the hessian matrix H_0 (Cholesky factor).
		yDotS = it.yDotS = InnerProduct(it.y, it.s);
		yy = InnerProduct(it.y, it.y);

		/*
		Recursive formula to compute dir = -(H \cdot g).
		This is described in page 779 of:
		Jorge Nocedal.
		Updating Quasi-Newton Matrices with Limited Storage.
		Mathematics of Computation, Vol. 35, No. 151,
		pp. 773--782, 1980.
		*/
		bound = Min(m, k);
		++k;
		end = (end + 1) % m;

		// Compute the steepest direction. 
		d = Apply(std::negate<double>(), g);
		j = end;
		for (i = 0; i < bound; ++i) 
		{
			j = (j + m - 1) % m; 
			auto& it = lm[j];
			// \alpha_{j} = \rho_{j} s^{t}_{j} \cdot q_{k+1}
			it.alpha = InnerProduct(it.s, d) / it.yDotS;
			// q_{i} = q_{i+1} - \alpha_{i} y_{i}
			Transform(&d, it.y, LinearIncrement(-it.alpha));
		}
		d *= yDotS / yy;

		for (i = 0; i < bound; ++i) 
		{
			auto& it = lm[j];
			// \beta_{j} = \rho_{j} y^t_{j} \cdot \gamma_{i}
			const double beta = InnerProduct(it.y, d) / it.yDotS;
			// \gamma_{i+1} = \gamma_{i} + (\alpha_{j} - \beta_{j}) s_{j}
			Transform(&d, it.s, LinearIncrement(it.alpha - beta));
			j = (j + 1) % m;   
		}

		// Now the search direction d is ready. We try step = 1 first.
		step = 1.0;
	}
}


static int line_search_backtracking
	(Vector_<>* x,
	double* f,
	Vector_<>* g,
	const Vector_<>& s,
	double* stp,
	const Vector_<>& xp,
	const Vector_<>& gp,
	const BFGS::Func_& func,
	const BFGSControls_& param)
{
	static const double DEC = 0.5, INC = 2.1;

	// Check the input parameters for errors. 
	REQUIRE(*stp > 0.0, "Nonpositive step in linesearch");

	// Compute the initial gradient in the search direction.
	const double dginit = InnerProduct(*g, s);
	REQUIRE(dginit <= 0, "Gradient is not a descent direction");

	/* The initial value of the objective function. */
	const double finit = *f;
	const double dgtest = param.ftol_ * dginit;

	for (int count = 1; ; ++count) 
	{
		Transform(xp, s, LinearIncrement(*stp), x);
		*f = func(*x, g, *stp);

		double width = DEC;
		if (*f <= finit + *stp * dgtest) 
		{
			// The sufficient decrease condition (Armijo condition)
			if (param.linesearch_ == LinesearchMethod_::Value_::ARMIJO) 
				return count; // Exit with the Armijo condition. 

			/* Check the Wolfe condition. */
			const double dg = InnerProduct(*g, s);
			if (dg < param.wolfe_ * dginit) 
				width = INC;
			else 
			{
				if (param.linesearch_ == LinesearchMethod_::Value_::WOLFE) 
					return count;	// Exit with the regular Wolfe condition.

				/* Check the strong Wolfe condition. */
				if (dg <= -param.wolfe_ * dginit) 
					return count;	// Exit with the strong Wolfe condition.
			}
		}

		REQUIRE(*stp >= param.min_step_, "Step too small in L-BFGS");
		REQUIRE(*stp <= param.max_step_, "Step too large in L-BFGS");
		REQUIRE(count < param.max_linesearch_, "Exhausted iterations in linesearch");

		*stp *= width;
	}
}


static int line_search_morethuente
	(Vector_<>* x,
	double* f,
	Vector_<>* g,
	const Vector_<>& s,
	double* stp,
	const Vector_<>& xp,
	const Vector_<>& gp,
	const BFGS::Func_& func,
	const BFGSControls_& param)
{
	REQUIRE(*stp > 0.0, "Invalid step input to linesearch");

	/* Compute the initial gradient in the search direction. */
	const double dginit = InnerProduct(*g, s);
	REQUIRE(dginit <= 0, "Gradient is not a descent direction");

	/* Initialize local variables. */
	int brackt = 0;
	bool stage1 = true;
	const double finit = *f;
	const double dgtest = param.ftol_ * dginit;
	double width = param.max_step_ - param.min_step_;
	double prev_width = 2.0 * width;

	/*
	The variables stx, fx, dgx contain the values of the step,
	function, and directional derivative at the best step.
	The variables sty, fy, dgy contain the value of the step,
	function, and derivative at the other endpoint of
	the interval of uncertainty.
	The variables stp, f, dg contain the values of the step,
	function, and derivative at the current step.
	*/
	double stx = 0., sty = 0.0;
	double fx = finit, fy = finit;
	double dgx = dginit, dgy = dginit;

	for (int count = 0;;) 
	{
		// Set the minimum and maximum steps to correspond to the present interval of uncertainty
		const double stmin = brackt ? Min(stx, sty) : stx;
		const double stmax = brackt ? Max(stx, sty) : *stp + 4.0 * (*stp - stx);

		/* Clip the step in the range of [stpmin, stpmax]. */
		*stp = Min(param.max_step_, Max(param.min_step_, *stp));

		// If an unusual termination is to occur then let stp be the lowest point obtained so far.
		if ((brackt && ((*stp <= stmin || stmax <= *stp) || count + 1 >= param.max_linesearch_))
				|| (brackt && (stmax - stmin <= std::numeric_limits<double>::epsilon() * stmax))) 
			*stp = stx;

		//	Compute the current value of x: x <- x + (*stp) * s.
		Transform(xp, s, LinearIncrement(*stp), x);

		*f = func(*x, g, *stp);
		double dg = InnerProduct(*g, s);

		const double ftest1 = finit + *stp * dgtest;
		++count;

		/* Test for errors and convergence. */
		REQUIRE(*stp < param.max_step_ || *f > ftest1 || dg > dgtest, "Step is too large in linesearch");
		REQUIRE(*stp > param.min_step_ || (*f <= ftest1 && dg < dgtest), "Step is too small in linesearch");
		if (brackt)
		{
			if (*stp <= stmin && param.halt_on_stall_)
				return count;	// pretend we were successful
			REQUIRE(*stp > stmin && *stp < stmax, "Rounding errors prevent further progress");
			REQUIRE(stmax - stmin > std::numeric_limits<double>::epsilon() * stmax, "Bracket width is too small in linesearch");
		}
		REQUIRE(count < param.max_linesearch_, "Exhausted iterations in linesearch");
		if (*f <= ftest1 && fabs(dg) <= param.gtol_ * (-dginit)) 
			return count;	// The sufficient decrease condition and the directional derivative condition hold.

		/*
		In the first stage we seek a step for which the modified
		function has a nonpositive value and nonnegative derivative.
		*/
		if (stage1 && *f <= ftest1 && Min(param.ftol_, param.gtol_) * dginit <= dg) 
			stage1 = false;

		/*
		A modified function is used to predict the step only if
		we have not obtained a step for which the modified
		function has a nonpositive function value and nonnegative
		derivative, and if a lower function value has been
		obtained but the decrease is not sufficient.
		*/
		if (stage1 && ftest1 < *f && *f <= fx) 
		{
			/* Define the modified function and derivative values. */
			double fm = *f - *stp * dgtest;
			double fxm = fx - stx * dgtest;
			double fym = fy - sty * dgtest;
			double dgm = dg - dgtest;
			double dgxm = dgx - dgtest;
			double dgym = dgy - dgtest;

			/*
			Call update_trial_interval() to update the interval of
			uncertainty and to compute the new step.
			*/
			update_trial_interval(
				&stx, &fxm, &dgxm,
				&sty, &fym, &dgym,
				stp, &fm, &dgm,
				stmin, stmax, &brackt
				);

			/* Reset the function and gradient values for f. */
			fx = fxm + stx * dgtest;
			fy = fym + sty * dgtest;
			dgx = dgxm + dgtest;
			dgy = dgym + dgtest;
		}
		else 
		{
			/*
			Call update_trial_interval() to update the interval of
			uncertainty and to compute the new step.
			*/
			update_trial_interval(
				&stx, &fx, &dgx,
				&sty, &fy, &dgy,
				stp, f, &dg,
				stmin, stmax, &brackt
				);
		}

		//	Force a sufficient decrease in the interval of uncertainty.
		if (brackt) 
		{
			if (0.66 * prev_width <= fabs(sty - stx)) 
				*stp = stx + 0.5 * (sty - stx);
			prev_width = width;
			width = fabs(sty - stx);
		}
	}
	assert(!"Unreachable code in More-Thuente linesearch");
}

int LinesearchMethod_::LineSearch
	(Vector_<>* x, 
	 double* f, 
	 Vector_<>* g, 
	 const Vector_<>& s, 
	 double* stp, 
	 const Vector_<>& xp, 
	 const Vector_<>& gp, 
	 const BFGS::Func_& func, 
	 const BFGSControls_& controls) 
const
{
	switch (Switch())
	{
	case LinesearchMethod_::Value_::MORE_THUENTE:
		return line_search_morethuente(x, f, g, s, stp, xp, gp, func, controls);
	default:
		assert(*this == controls.linesearch_);	// backtracking linesearch switches on method in controls
		return line_search_backtracking(x, f, g, s, stp, xp, gp, func, controls);
	}
}

// Define the local variables for computing minimizers.
struct PolyMin_
{
	double a, d, gamma, theta, p, q, r, s;

	/**
	* Find a minimizer of an interpolated cubic function.
	*  @param  cm      The minimizer of the interpolated cubic.
	*  @param  u       The value of one point, u.
	*  @param  fu      The value of f(u).
	*  @param  du      The value of f'(u).
	*  @param  v       The value of another point, v.
	*  @param  fv      The value of f(v).
	*  @param  du      The value of f'(v).
	*/
	void cubic(double& cm, const double u, const double fu, const double du, const double v, const double fv, const double dv)
	{
		d = (v)-(u);
		theta = ((fu)-(fv)) * 3 / d + (du)+(dv);
		p = fabs(theta);
		q = fabs(du);
		r = fabs(dv);
		s = Max(p, q, r);
		/* gamma = s*sqrt((theta/s)**2 - (du/s) * (dv/s)) */
		a = theta / s;
		gamma = s * sqrt(a * a - ((du) / s) * ((dv) / s));
		if ((v) < (u)) gamma = -gamma;
		p = gamma - (du)+theta;
		q = gamma - (du)+gamma + (dv);
		r = p / q;
		(cm) = (u)+r * d;
	}
	/**
	* Find a minimizer of an interpolated cubic function.
	*  @param  cm      The minimizer of the interpolated cubic.
	*  @param  u       The value of one point, u.
	*  @param  fu      The value of f(u).
	*  @param  du      The value of f'(u).
	*  @param  v       The value of another point, v.
	*  @param  fv      The value of f(v).
	*  @param  du      The value of f'(v).
	*  @param  xmin    The maximum value.
	*  @param  xmin    The minimum value.
	*/
	void cubic2(double& cm, const double u, const double fu, const double du, const double v, const double fv, const double dv, const double xmin, const double xmax)
	{
		d = (v)-(u);
		theta = ((fu)-(fv)) * 3 / d + (du)+(dv);
		p = fabs(theta);
		q = fabs(du);
		r = fabs(dv);
		s = Max(p, q, r);
		/* gamma = s*sqrt((theta/s)**2 - (du/s) * (dv/s)) */
		a = theta / s;
		gamma = s * sqrt(Max(0.0, a * a - ((du) / s) * ((dv) / s)));
		if ((u) < (v))
			gamma = -gamma;
		p = gamma - (dv)+theta;
		q = gamma - (dv)+gamma + (du);
		r = p / q;
		if (r < 0. && gamma != 0.)
			(cm) = (v)-r * d;
		else if (a < 0)
			(cm) = (xmax);
		else
			(cm) = (xmin);
	}

	/**
	* Find a minimizer of an interpolated quadratic function.
	*  @param  qm      The minimizer of the interpolated quadratic.
	*  @param  u       The value of one point, u.
	*  @param  fu      The value of f(u).
	*  @param  du      The value of f'(u).
	*  @param  v       The value of another point, v.
	*  @param  fv      The value of f(v).
	*/
	void quard(double& qm, double u, double fu, double du, double v, double fv)
	{
		a = (v)-(u);
		(qm) = (u)+(du) / (((fu)-(fv)) / a + (du)) / 2 * a;
	}

	/**
	* Find a minimizer of an interpolated quadratic function.
	*  @param  qm      The minimizer of the interpolated quadratic.
	*  @param  u       The value of one point, u.
	*  @param  du      The value of f'(u).
	*  @param  v       The value of another point, v.
	*  @param  dv      The value of f'(v).
	*/
	void quard2(double& qm, double u, double du, double v, double dv)
	{
		a = (u)-(v);
		(qm) = (v)+(dv) / ((dv)-(du)) * a;
	}
};
/**
* Update a safeguarded trial value and interval for line search.
*
*  The parameter x represents the step with the least function value.
*  The parameter t represents the current step. This function assumes
*  that the derivative at the point of x in the direction of the step.
*  If the bracket is set to true, the minimizer has been bracketed in
*  an interval of uncertainty with endpoints between x and y.
*
*  @param  x       The pointer to the value of one endpoint.
*  @param  fx      The pointer to the value of f(x).
*  @param  dx      The pointer to the value of f'(x).
*  @param  y       The pointer to the value of another endpoint.
*  @param  fy      The pointer to the value of f(y).
*  @param  dy      The pointer to the value of f'(y).
*  @param  t       The pointer to the value of the trial value, t.
*  @param  ft      The pointer to the value of f(t).
*  @param  dt      The pointer to the value of f'(t).
*  @param  tmin    The minimum value for the trial value, t.
*  @param  tmax    The maximum value for the trial value, t.
*  @param  brackt  The pointer to the predicate if the trial value is
*                  bracketed.
*  @retval int     Status value. Zero indicates a normal termination.
*
*  @see
*      Jorge J. More and David J. Thuente. Line search algorithm with
*      guaranteed sufficient decrease. ACM Transactions on Mathematical
*      Software (TOMS), Vol 20, No 3, pp. 286-307, 1994.
*/
static void update_trial_interval(
	double *x,
	double *fx,
	double *dx,
	double *y,
	double *fy,
	double *dy,
	double *t,
	double *ft,
	double *dt,
	const double tmin,
	const double tmax,
	int *brackt)
{
	int bound;
	int dsign = *dt * *dx < 0.0;
	double mc; /* minimizer of an interpolated cubic. */
	double mq; /* minimizer of an interpolated quadratic. */
	double newt;   /* new trial value. */
	PolyMin_ minimize;

	/* Check the input parameters for errors. */
	if (*brackt) 
	{
		REQUIRE(*t > Min(*x, *y) && *t < Max(*x, *y), "Trial value is not bracketed");
		REQUIRE(*dx * (*t - *x) < 0, "Gradient does not indicate a descent direction");
		REQUIRE(tmax >= tmin, "Invalid bracketing interval");
	}

	//	Trial value selection
	if (*fx < *ft) 
	{
		/*
		Case 1: a higher function value.
		The minimum is brackt. If the cubic minimizer is closer
		to x than the quadratic one, the cubic one is taken, else
		the average of the minimizers is taken.
		*/
		*brackt = 1;
		bound = 1;
		minimize.cubic(mc, *x, *fx, *dx, *t, *ft, *dt);
		minimize.quard(mq, *x, *fx, *dx, *t, *ft);
		if (fabs(mc - *x) < fabs(mq - *x)) {
			newt = mc;
		}
		else {
			newt = mc + 0.5 * (mq - mc);
		}
	}
	else if (dsign) {
		/*
		Case 2: a lower function value and derivatives of
		opposite sign. The minimum is brackt. If the cubic
		minimizer is closer to x than the quadratic (secant) one,
		the cubic one is taken, else the quadratic one is taken.
		*/
		*brackt = 1;
		bound = 0;
		minimize.cubic(mc, *x, *fx, *dx, *t, *ft, *dt);
		minimize.quard2(mq, *x, *dx, *t, *dt);
		if (fabs(mc - *t) > fabs(mq - *t)) {
			newt = mc;
		}
		else 
			newt = mq;
	}
	else if (fabs(*dt) < fabs(*dx)) 
	{
		/*
		Case 3: a lower function value, derivatives of the
		same sign, and the magnitude of the derivative decreases.
		The cubic minimizer is only used if the cubic tends to
		infinity in the direction of the minimizer or if the minimum
		of the cubic is beyond t. Otherwise the cubic minimizer is
		defined to be either tmin or tmax. The quadratic (secant)
		minimizer is also computed and if the minimum is brackt
		then the the minimizer closest to x is taken, else the one
		farthest away is taken.
		*/
		bound = 1;
		minimize.cubic2(mc, *x, *fx, *dx, *t, *ft, *dt, tmin, tmax);
		minimize.quard2(mq, *x, *dx, *t, *dt);
		if (*brackt) {
			if (fabs(*t - mc) < fabs(*t - mq)) {
				newt = mc;
			}
			else 
				newt = mq;
		}
		else {
			if (fabs(*t - mc) > fabs(*t - mq)) {
				newt = mc;
			}
			else 
				newt = mq;
		}
	}
	else 
	{
		/*
		Case 4: a lower function value, derivatives of the
		same sign, and the magnitude of the derivative does
		not decrease. If the minimum is not brackt, the step
		is either tmin or tmax, else the cubic minimizer is taken.
		*/
		bound = 0;
		if (*brackt) {
			minimize.cubic(newt, *t, *ft, *dt, *y, *fy, *dy);
		}
		else if (*x < *t) {
			newt = tmax;
		}
		else {
			newt = tmin;
		}
	}

	/*
	Update the interval of uncertainty. This update does not
	depend on the new step or the case analysis above.

	- Case a: if f(x) < f(t),
	x <- x, y <- t.
	- Case b: if f(t) <= f(x) && f'(t)*f'(x) > 0,
	x <- t, y <- y.
	- Case c: if f(t) <= f(x) && f'(t)*f'(x) < 0,
	x <- t, y <- x.
	*/
	if (*fx < *ft) {
		/* Case a */
		*y = *t;
		*fy = *ft;
		*dy = *dt;
	}
	else {
		/* Case c */
		if (dsign) {
			*y = *x;
			*fy = *fx;
			*dy = *dx;
		}
		/* Cases b and c */
		*x = *t;
		*fx = *ft;
		*dx = *dt;
	}

	/* Clip the new trial value in [tmin, tmax]. */
	if (tmax < newt) newt = tmax;
	if (newt < tmin) newt = tmin;

	/*
	Redefine the new trial value if it is close to the upper bound
	of the interval.
	*/
	if (*brackt && bound) {
		mq = *x + 0.66 * (*y - *x);
		if (*x < *y) {
			if (mq < newt) newt = mq;
		}
		else {
			if (newt < mq) newt = mq;
		}
	}

	/* Return the new trial value. */
	*t = newt;
}



