
#include "__Platform.h"
#include "Globals.h"

#include "CellUtils.h"
#include "Index.h"
#include "IndexParse.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Fixings_Store
   Stores historical fixings
&inputs
index is string
   The name of the index for which fixings should be stored
fixings is cell[][]
	&$.Empty() || $.Cols() == 2\$ should have two columns (fixing times, fixing values)
	&$.Empty() || AllOf($.Col(0), Cell::TypeCheck_().DateTime())\First column of $ should be fixing times
	&$.Empty() || AllOf($.Col(1), Cell::TypeCheck_().Number())\First column of $ should be numeric fixings
	The historical fixings to store
&optional
append is boolean (true)
	If false, old fixings for this index will be erased.  In any case, old fixings will be overwritten by new fixings on the same date
&outputs
n_stored is integer
	The number of fixings now stored for the given index
-IF-------------------------------------------------------------------------*/

   void Fixings_Store
      (const String_& index_name,
	   const Matrix_<Cell_>& fixings,
	   bool append,
	   int* n_stored)
   {
	   FixHistory_ temp;
	   const int n = fixings.Rows();
	   temp.vals_.Resize(n);
	   for (int ii = 0; ii < n; ++ii)
		   temp.vals_[ii] = make_pair(Cell::ToDateTime(fixings(ii, 0)), Cell::ToDouble(fixings(ii, 1)));
	   // always store under the index canonical name
	   Handle_<Index_> index(Index::Parse(index_name));
	   *n_stored = XGLOBAL::StoreFixings(index->Name(), temp, append);
   }
}  // leave local

#include "MG_Fixings_Store_public.inc"

