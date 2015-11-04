
// a Storable_ whose role is to hold other Storables_

#pragma once

#include <map>
#include "Vectors.h"
#include "Storable.h"

/*IF--------------------------------------------------------------------------
storable Bag
	Holder for storable objects
manual
&members
name is ?string
contents is *handle
	Objects in the bag
keys is ?string[]
	Keys of the map in the bag
-IF-------------------------------------------------------------------------*/

struct Bag_ : Storable_
{
	std::multimap<String_, Handle_<Storable_>> contents_;	// Bag_ is very floppy, can be manipulated at will (e.g. by Auditor_) -- Handle_<Bag_> has fixed contents
	Bag_(const String_& name, const std::multimap<String_, Handle_<Storable_>>& objects) : Storable_("Bag", name), contents_(objects) {}
	void Write(Archive::Store_& dst) const override;
};

