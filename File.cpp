
#include "Platform.h"
#include "File.h"
#include <fstream>
#include "Strict.h"

#include "Vectors.h"

void File::Read
	(const String_& filename,
	 Vector_<String_>* dst)
{
	std::ifstream src(filename.c_str()); // C++ flaw -- fstream demands standard traits
	char buf[2048];
	while (src.getline(buf, 2048))
	{
		dst->emplace_back(buf);
	}
}

