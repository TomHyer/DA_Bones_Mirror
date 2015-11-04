
#include "__Platform.h"
#include "Splat.h"
#include "Strict.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Object_Display
   Show contents of a storable object
&inputs
object is handle
   Object to query
&outputs
data is cell[][]
   Data contained in the object
-IF-------------------------------------------------------------------------*/

   void Object_Display
      (const Handle_<Storable_>& object,
       Matrix_<Cell_>* data)
   {
      *data = Splat(*object);
   }

/*IF--------------------------------------------------------------------------
public Object_Build
   Reconstruct a storable object
&inputs
data is cell[][]
   Data contained in the object
&optional
quiet is boolean (false)
   If true, non-fatal errors such as unexpected child nodes will be ignored
&outputs
object is handle
   Object reconstituted from the data
-IF-------------------------------------------------------------------------*/

   void Object_Build
      (const Matrix_<Cell_>& data,
       bool quiet,
       Handle_<Storable_>* object)
   {
      *object = UnSplat(data, quiet);
   }
}  // leave local

#include "MG_Object_Display_public.inc"
#include "MG_Object_Build_public.inc"
