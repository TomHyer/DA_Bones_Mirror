
#include "__Platform.h"
#include "Box.h"
#include "Strict.h"
#include "MatrixUtils.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Box_New
   Create a storable object containing a block of data
&inputs
contents is cell[][]
   Data to enclose
&optional
name is string
   A name for the object being created
&outputs
object is handle Box
   The new object
-IF-------------------------------------------------------------------------*/

   void Box_New
      (const Matrix_<Cell_>& contents,
      const String_& name,
      Handle_<Box_>* object)
   {
      object->reset(new Box_(name, contents));
   }

/*IF--------------------------------------------------------------------------
public Box_Unbox
   Recover data from a box
&inputs
box is handle Box
   Object to query
&optional
format is string
	Instructions for display
&outputs
contents is cell[][]
   Contents of the box
-IF-------------------------------------------------------------------------*/

   void Box_Unbox
      (const Handle_<Box_>& box,
	   const String_& format,
       Matrix_<Cell_>* contents)
   {
      *contents = box->contents_;
	  if (!format.empty())
		  Matrix::Format(Vector_<const Matrix_<Cell_>*>(1, contents), format).Swap(contents);
   }
}

#include "MG_Box_New_public.inc"
#include "MG_Box_Unbox_public.inc"

