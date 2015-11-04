
// block-diagonal sparse matrices

#pragma once

namespace Sparse
{
   class Square_;

   Square_* NewBlockDiagonal
      (const Vector_<Handle_<Square_>>& blocks);
}

