
// Archive with underlying JSON document

#pragma once

class String_;
class Storable_;

namespace JSON
{
	Handle_<Storable_> ReadString(const String_& src, bool quiet);
	Handle_<Storable_> ReadFile(const String_& filename, bool quiet);
	void WriteFile(const Storable_& object, const String_& filename);
	String_ WriteString(const Storable_& object);
}

