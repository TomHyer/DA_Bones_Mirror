
#include "Platform.h"
#include "Archive.h"
#include <map>
#include "Strict.h"

#include "Strings.h"
#include "Exceptions.h"
using std::map;

String_ Archive::VECTOR = "REPEATED";

Archive::Store_::~Store_()
{	}

Archive::View_::~View_()
{	}

Archive::Reader_::~Reader_()
{	}

Archive::Store_& Archive::Store_::Element(int index)
{
	return Child(String::FromInt(index));
}

const Archive::View_& Archive::View_::Element(int index) const
{
	return Child(String::FromInt(index));
}
bool Archive::View_::HasElement(int index) const
{
	return HasChild(String::FromInt(index));
}

namespace
{
	map<String_, const Archive::Reader_*> & TheBuilders()
	{
		RETURN_STATIC(map<String_, const Archive::Reader_*>);
	}
}	// leave local

void Archive::Register(const String_& type, const Archive::Reader_* d_type)
{
	TheBuilders().insert(make_pair(type, d_type));
}

Handle_<Storable_> Archive::Extract
   (const Archive::View_& src,
    Built_& built)
{
   Handle_<Storable_>& retval = src.Known(built);
   if (!retval)
   {
      const String_& type = src.Type();
      REQUIRE(!type.empty(), "No type supplied; can't extract a handle");
      NOTICE(type);
      auto pb = TheBuilders().equal_range(type);
      REQUIRE(pb.first != pb.second, "Type has no builder");
      REQUIRE(pb.first == --pb.second, "Builder is not unique");
      retval.reset(pb.first->second->Build(src, built));
   }
   return retval;
}

void Archive::Utils::SetStorable
   (Archive::Store_& dst,
    const String_& name,
    const Storable_& value)
{
   auto& child = dst.Child(name);
   if (child.StoreRef(&value))
	   child.Done();
   else
      value.Write(child);
}
   