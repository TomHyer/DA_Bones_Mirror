
#include "Platform.h"
#include "Box.h"
#include "Strict.h"

#include "Archive.h"

namespace
{
#include "MG_Box_Write.inc"
#include "MG_Box_Read.inc"
}

void Box_::Write(Archive::Store_& dst) const
{
   Box::XWrite(dst, name_, contents_);
}
