
#include "Platform.h"
#include "Model.h"
#include "Strict.h"

// implement the generic mutator in terms of single slides
Model_* Model_::Mutant_Model
	(const String_& new_name,
	 const Vector_<Handle_<Slide_> >& slides)
const
{
	std::unique_ptr<Model_> retval(Mutant_Model(&new_name));
	for (const auto& s : slides)
	{
		std::unique_ptr<Model_> temp(retval->Mutant_Model(nullptr, s.get()));
		std::swap(retval, temp);
	}
	return retval.release();
}

