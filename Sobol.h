
// Sobol sequences, conforming to general quasi-random interface

#pragma once

#include "QuasiRandom.h"

namespace QuasiRandom
{
	SequenceSet_* NewSobol(int size, int i_path);
}

