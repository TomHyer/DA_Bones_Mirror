
#include "Platform.h"
#include "Environment.h"
#include "Strict.h"

Environment::Entry_::~Entry_()
{	}

Environment_::~Environment_()
{	}

Environment_::IterImp_::~IterImp_()
{	}

void Environment_::Iterator_::operator++()
{
	if (IsValid())
		imp_.reset(imp_->Next());
}