
#include "__Platform.h"
#include "Bag.h"
#include "Strict.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Bag_New
   Create a storable object containing other storable objects
&inputs
contents is handle[]
   Objects to hold
&optional
name is string
   A name for the object being created
keys is string[]
   &$.empty() || $.size() == contents.size()\$ and contents must have same size
   Keys to index objects within the bag -- by default, object names are used
&outputs
object is handle Bag
   The new object
-IF-------------------------------------------------------------------------*/

   void Bag_New
      (const Vector_<Handle_<Storable_>>& objects,
       const String_& name,
       const Vector_<String_>& keys,
       Handle_<Bag_>* bag)
   {
      std::multimap<String_, Handle_<Storable_>> contents;
      for (int ii = 0; ii < objects.size(); ++ii)
         contents.insert(make_pair(keys.empty() ? objects[ii]->name_ : keys[ii], objects[ii]));

      bag->reset(new Bag_(name, contents));
   }

/*IF--------------------------------------------------------------------------
public Bag_Contents
   Recover the objects inside a bag
&inputs
bag is handle Bag
   Object to query
&outputs
keys is string[]
   Keys assigned to the bag's objects
objects is handle[]
   The bag's contents
-IF-------------------------------------------------------------------------*/

   void Bag_Contents
      (const Handle_<Bag_>& bag,
       Vector_<String_>* keys,
       Vector_<Handle_<Storable_>>* objects)
   {
	   for (const auto& kv : bag->contents_)
      {
         keys->push_back(kv.first);
         objects->push_back(kv.second);
      }
   }
}  // leave local

#include "MG_Bag_New_public.inc"
#include "MG_Bag_Contents_public.inc"

