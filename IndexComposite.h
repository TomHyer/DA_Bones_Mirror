
// linear combinations of underlying indices

#pragma once

#include "Index.h"

namespace Index
{
	class Composite_ : public Index_
	{
	public:
		typedef pair<Handle_<Index_>, double> component_t;
	private:
		Vector_<component_t> components_;
		double Fixing(_ENV, const DateTime_& time) const override;
		String_ Name() const override;
	};
}

