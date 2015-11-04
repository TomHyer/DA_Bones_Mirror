
// parse index from human-readable descriptive string

#pragma once

class Index_;
class String_;

namespace Index
{
   Index_* Parse(const String_& name);
   typedef Index_*(*parser_t)(const String_&);
   void RegisterParser(parser_t func);
   Handle_<Index_> Clone(const Index_& src);	// in case we need a handle but have only a reference
}

