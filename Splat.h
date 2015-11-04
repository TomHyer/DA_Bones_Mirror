
#pragma once

#include "Matrix.h"

struct Cell_;
class Storable_;

Matrix_<Cell_> Splat(const Storable_& src);
Handle_<Storable_> UnSplat(const Matrix_<Cell_>& src, bool quiet);
Handle_<Storable_> UnSplatFile(const String_& filename, bool quiet);

