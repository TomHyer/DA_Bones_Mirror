
#include "Platform.h"
#include "Bag.h"
#include "Strict.h"

#include "Archive.h"
#include "Maps.h"
#include "Algorithms.h"

namespace
{
#include "MG_Bag_Write.inc"
#include "MG_Bag_Read.inc"

	Storable_* Bag::Reader_::Build() const
	{
		return new Bag_(name_, ZipToMultimap(keys_, contents_));
	}
}

void Bag_::Write(Archive::Store_& dst) const
{
   Bag::XWrite(dst, name_, MapValues(contents_), Keys(contents_));
}
