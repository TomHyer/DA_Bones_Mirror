
#include "__Platform.h"
#include "Strict.h"

#include "Interp.h"
#include "InterpCubic.h"
#include "Quadrature.h"
#include "Smooth.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Interp1_New_Linear
   Create a linear interpolator
&inputs
name is string
   A name for the object being created
x is number[]
   &IsMonotonic(x)\$ values must be in ascending order
   The x-values (abcissas)
y is number[]
   &$.size() == x.size()\x and y must have the same size
   The values of f(x) at each x
&outputs
f is handle Interp1
   The interpolator
-IF-------------------------------------------------------------------------*/

   void Interp1_New_Linear
      (const String_& name,
       const Vector_<>& x,
       const Vector_<>& y,
       Handle_<Interp1_>* f)
   {
      f->reset(new Interp1Linear_(name, x, y));
   }

/*IF--------------------------------------------------------------------------
public Interp1_New_Linear_Smoothed
   Create a linear interpolator
&inputs
name is string
   A name for the object being created
x is number[]
   &IsMonotonic(x)\$ values must be in ascending order
   The x-values (abcissas)
y is number[]
   &$.size() == x.size()\x and y must have the same size
   The values of f(x) at each x
smoothing is number
	&$ >= 0.0
	The weight to put on smoothness of the interpolating function
&optional
fit_weights is number[]
	&$.empty() || $.size() == x.size()\must have one $ for each y
	The weight to attach to accuracy of fit for each y_i; default is 1.0 for all
&outputs
f is handle Interp1
   The interpolator
-IF-------------------------------------------------------------------------*/

   void Interp1_New_Linear_Smoothed
      (const String_& name,
       const Vector_<>& x,
       const Vector_<>& y,
	   double smoothing,
	   const Vector_<>& fit_weights,
       Handle_<Interp1_>* f)
   {
	   Vector_<> z = SmoothedVals(x, y, fit_weights, smoothing);
      f->reset(new Interp1Linear_(name, x, z));
   }

/*IF--------------------------------------------------------------------------
public Interp1_New_Cubic
   Create a cubic-spline interpolator
&inputs
name is string
   A name for the object being created
x is number[]
   &IsMonotonic($)\$ values must be in ascending order
   The x-values (abcissas) at which the function value is specified
y is number[]
   &$.size() == x.size()\x and y must have the same size
   The values of f(x) at each x
&optional
boundary_order is integer[]
   &$.size() <= 2\Can only specify two boundary conditions
   &$.empty() || ($.front() > 0 && $.back() > 0 && $.front() <= 3 && $.back() <= 3)\Boundary order must be in the range (0, 3]
   The order of the derivatives specified at the boundary (default is 3); can be a two-element vector of (left, right), or a single value for both
boundary_value is number[]
   &$.size() <= 2\Can only specify two boundary conditions
   &$.empty() || !boundary_order.empty()\Can't specify boundary value without specifying order
   The value of whichever derivative is specified at the boundary (default is 0.0); can be a two-element vector of (left, right), or a single number for both
&outputs
f is handle Interp1
   The interpolator
-IF-------------------------------------------------------------------------*/

   void Interp1_New_Cubic
      (const String_& name,
       const Vector_<>& x,
       const Vector_<>& y,
       const Vector_<int>& boundary_order,
       const Vector_<>& boundary_value,
       Handle_<Interp1_>* f)
   {
      Interp::Boundary_ left(3, 0.0), right(3, 0.0);
      if (!boundary_order.empty())
      {
         left.order_ = boundary_order.front();
         right.order_ = boundary_order.back();
         if (!boundary_value.empty())
         {
            left.value_ = boundary_value.front();
            right.value_ = boundary_value.back();
         }
      }
      f->reset(Interp::NewCubic(name, x, y, left, right));
   }

/*IF--------------------------------------------------------------------------
public Interp1_Get
   Interpolate a value at specified abcissas
&inputs
f is handle Interp1
   The interpolant function
x is number[]
   The x-values (abcissas)
&outputs
y is number[]
   The interpolated function values at x-values
-IF-------------------------------------------------------------------------*/

   double CheckedInterp(const Interp1_& f, double x)
   {
      REQUIRE(f.IsInBounds(x), "X (= " + std::to_string(x) + ") is outside interpolation domain");
      return f(x);
   }

   void Interp1_Get
      (const Handle_<Interp1_>& f,
      const Vector_<>& x,
      Vector_<>* y)
   {
      *y = Apply([&](double x_i){return CheckedInterp(*f, x_i); }, x);
   }

/*IF--------------------------------------------------------------------------
public Interp1_Integrate
   Integrate an interpolated function (to test integrator)
&inputs
f is handle Interp1
   The interpolant function
lo is number
   The low end of the range
hi is number
   The high end of the range
&optional
n is integer (100)
   &$ > 0\Number of evaluations must be positive
   The number of evaluations to use
&outputs
integral is number
   The integrated interpolated function
-IF-------------------------------------------------------------------------*/

   void Interp1_Integrate
      (const Handle_<Interp1_>& f_in,
       double lo,
       double hi,
       int n,
       double* integral)
   {
      auto f = [&](double x_i){ return CheckedInterp(*f_in, x_i); };
      QuadSimpson_<> task(n, lo, hi);
      while (!task.IsComplete())
         task.PutY(f(task.GetX()));
      *integral = task.Result();
   }
}   // leave local

#include "MG_Interp1_New_Linear_public.inc"
#include "MG_Interp1_New_Linear_Smoothed_public.inc"
#include "MG_Interp1_New_Cubic_public.inc"
#include "MG_Interp1_Get_public.inc"
#include "MG_Interp1_Integrate_public.inc"



#include "_Cookie.h"


extern "C" __declspec(dllexport) double ffi_Interp1_New_Linear
   (const char* f_name, int f_x, int f_y)
{
   Cookie::Output_ retval;
   const char* argName = 0;
   try
   {
      Cookie::Input_ get;
      Log::Write("Interp1_New_Linear");
      argName = "name (input #1)";
      const String_ name(f_name);
      argName = "x (input #2)";
      const Vector_<double> x = get.VectorDouble(f_x);
      argName = "y (input #3)";
      const Vector_<double> y = get.VectorDouble(f_y);
      argName = 0;

      Handle_<Interp1_> f;
      Interp1_New_Linear(name, x, y, &f);
      retval.Append(f);
      return retval.Finalize();
   }
   catch (std::exception& e)
   {
      return retval.Error(e.what(), argName);
   }
   catch (...)
   {
      return retval.Error("Unknown error", argName);
   }
}

extern "C" __declspec(dllexport) double ffi_Interp1_Get
   (int f_f, int f_x)
{
   Cookie::Output_ retval;
   const char* argName = 0;
   try
   {
      Cookie::Input_ get;
      Log::Write("Interp1_Get");
      argName = "f (input #1)";
      const Handle_<Interp1_> f = get.Handle<Interp1_>(f_f);
      argName = "x (input #2)";
      const Vector_<double> x = get.VectorDouble(f_x);
      argName = 0;

      Vector_<double> y;
      Interp1_Get(f, x, &y);
      retval.Append(y);
      return retval.Finalize();
   }
   catch (std::exception& e)
   {
      return retval.Error(e.what(), argName);
   }
   catch (...)
   {
      return retval.Error("Unknown error", argName);
   }
}


