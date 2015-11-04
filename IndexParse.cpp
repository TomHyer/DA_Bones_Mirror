
#include "Platform.h"
#include "IndexParse.h"
#include <map>
#include "Strict.h"

#include "Strings.h"
#include "Exceptions.h"
#include "IndexComposite.h"

namespace
{
	// stubbed out, sorry
	Index_* ParseSuperShort(const String_&) { return nullptr; }
	
	std::map<String_, Index::parser_t>& TheIndexParsers()
	{
      RETURN_STATIC(std::map<String_, Index::parser_t>);
	}

	Index_* ParseSingle(const String_& name)
	{
		auto stop = name.find_first_of(":[");
		if (stop == String_::npos)
			return ParseSuperShort(name);

      const String_ ac = name.substr(0, stop);
		auto pp = TheIndexParsers().find(ac);
		REQUIRE(pp != TheIndexParsers().end(), "No parser for '" + name + "'");
		return (*pp->second)(name);
	}

   // stubbed out, sorry
   Index::Composite_* ParseComposite(const String_&)
   {
      return nullptr;
   }
}

Index_* Index::Parse(const String_& name)
{
	if (Composite_* test = ParseComposite(name))
		return test;
	return ParseSingle(name);
}

Handle_<Index_> Index::Clone(const Index_& src)
{
	return Parse(src.Name());
}

