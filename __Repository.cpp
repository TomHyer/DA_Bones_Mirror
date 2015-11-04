
#include "__Platform.h"
#include "Strict.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Repository_Erase
	Erase objects from repository
&inputs
objects is handle[]
	The objects to remove(their handles will be invalidated)
&outputs
num_erased is integer
	The number of objects removed
-IF-------------------------------------------------------------------------*/

	void Repository_Erase
		(const Vector_<Handle_<Storable_>>& objects,
		 int* num_erased)
	{
		ENV_SEED_TYPE(ObjectAccess_);	// POSTPONED -- mark this function as taking _ENV input
		auto repo = Environment::Find<ObjectAccess_>(_env);
		assert(repo);

		*num_erased = 0;
		for (const auto& obj : objects)
			if (repo->Erase(*obj))
				++*num_erased;
	}

/*IF--------------------------------------------------------------------------
public Repository_Find
	Find existing objects in the repository
&inputs
match is string
	A pattern to match (by search)
&outputs
objects is handle[]
	The repository objects matching the pattern
-IF-------------------------------------------------------------------------*/

	void Repository_Find
		(const String_& pattern,
		 Vector_<Handle_<Storable_>>* objects)
	{
		ENV_SEED_TYPE(ObjectAccess_);	// POSTPONED -- mark this function as taking _ENV input
		auto repo = Environment::Find<ObjectAccess_>(_env);
		assert(repo);
		*objects = repo->Find(pattern);
		REQUIRE(!objects->empty(), "No objects found");
	}

/*IF--------------------------------------------------------------------------
public Repository_Size
	Number of objects in the repository
&outputs
size is integer
	The number of stored objects
-IF-------------------------------------------------------------------------*/

	void Repository_Size(int* size)
	{
		ENV_SEED_TYPE(ObjectAccess_);	// POSTPONED -- mark this function as taking _ENV input
		auto repo = Environment::Find<ObjectAccess_>(_env);
		assert(repo);
		*size = repo->Size();
	}
}	// leave local

#include "MG_Repository_Erase_public.inc"
#include "MG_Repository_Find_public.inc"
#include "MG_Repository_Size_public.inc"

