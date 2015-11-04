
#include "Platform.h"
#include "Strings.h"
#include <bitset>
#include "Strict.h"

#include "Vectors.h"
#include "Algorithms.h"
#include "Functionals.h"

Vector_<String_> String::Split
	(const String_& src,
	 char sep,
	 bool keep_empties)
{
	Vector_<String_> retval;
	auto start = src.begin();
	while (start != src.end())
	{
		auto loc = find(start, src.end(), sep);
		if (loc == src.end())
		{
			retval.emplace_back(start, src.end());
			start = src.end();
		}
		else
		{
			retval.emplace_back(start, loc);
			start = ++loc;	// ok to destroy loc in the process
		}
		if (retval.back().empty() && !keep_empties)
			retval.pop_back();
	}
	return retval;
}

bool String::IsNumber(const String_& src)
{
	try
	{
		(void)ToDouble(src);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

double String::ToDouble(const String_& src)
{
	return std::stod(src.c_str());	// C++ flaw -- stod demands standard traits
}

int String::ToInt(const String_& src)
{
	return std::stoi(src.c_str());
}

String_ String::FromDouble(double src)
{
	return std::to_string(src).c_str();
}
String_ String::FromInt(int src)
{
	return std::to_string(src).c_str();
}

namespace
{
	bool IsFluff(char c)
	{
		switch (c)
		{
		case ' ':
		case '\t':
		case '_':
			return true;
		default:
			return false;
		}
	}
}	// leave local

String_ String::Condensed(const String_& src)
{
	String_ retval;
	for (const auto& c : src)
	{
		if (!IsFluff(c))
			retval.push_back(static_cast<char>(toupper(c)));
	}
	return retval;
}

String_ String::NextName(const String_& name)
{
	if (name.empty())
		return "0";
	String_ retval(name);
	switch (retval.back())
	{
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
		++retval.back();
		return retval;
	case '9':
		retval.pop_back();
		return NextName(retval) + '0';
	default:
		return retval + '1';
	}
}

// compares compressed version of a string with already-compressed rhs
bool String::Equivalent(const String_& lhs, const char* rhs)
{
	struct Otiose_ : std::bitset<256>
	{
		Otiose_() { set(' '); set('\t'); set('_'); }
	};
	static const Otiose_ SKIP;

	auto p = lhs.begin();
	auto q = rhs;
	for (;;)
	{
		while (p != lhs.end() && SKIP[*p])
			++p;
		if (!*q || p == lhs.end())
			return !*q && p == lhs.end();
		if (!ci_traits::eq(*p, *q))
			return false;
		++p, ++q;
	}
}

String_ String::Uniquifier(const void* p)
{
   unsigned int ii = reinterpret_cast<unsigned int>(p);
   String_ retval;
   while (ii > 0)
   {
      const int c = ii % 36;
      retval.push_back(static_cast<char>(c > 9 ? (c - 10) + 'a' : c + '0'));
      ii /= 36;
   }
   return retval;
}
