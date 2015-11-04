
#include "Platform.h"
#include "Index.h"
#include <map>
#include "Strict.h"

#include "Fixings.h"
#include "Exceptions.h"
#include "DateTime.h"

Index_::~Index_()
{	}

struct FixingsAccess_ : Environment::Entry_
{
   Handle_<Fixings_> Fetch(const String_& index_name) const
   // stubbed out, sorry
   {
      return Handle_<Fixings_>();
   } 
};

double Index::PastFixing
	(_ENV, const String_& index_name,
	 const DateTime_& fixing_time, 
	 bool quiet)
{
	static const std::map<DateTime_, double> EMPTY;
   auto hist = Environment::Find<FixingsAccess_>(_env);
   REQUIRE(hist || quiet, "No fixings access");
   NOTICE(index_name);
   auto fixings = hist->Fetch(index_name);
	REQUIRE(fixings || quiet, "No fixings exist");
	NOTICE(fixing_time);

	auto vals = fixings ? fixings->vals_ : EMPTY;
	auto pf = vals.find(fixing_time);
	if (pf == vals.end())
	{
		REQUIRE(quiet, "No fixing for this time");
		return -DA::INFINITY;
	}
	return pf->second;
}

double Index_::Fixing(_ENV, const DateTime_& fixing_time) const 
{ 
	return Index::PastFixing(_env, Name(), fixing_time); 
}

