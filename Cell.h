
// holder for a generic scalar type

#pragma once

#include "DateTime.h"

namespace Cell
{
	enum class Type_ : char
	{
		EMPTY,
		BOOLEAN,
		NUMBER,
		DATE,
		DATETIME,
		STRING,
		N_TYPES
	};
}

struct Cell_
{
	typedef Cell::Type_ Type_;
	Type_ type_;
	bool b_;
	double d_;
	String_ s_;
	DateTime_ dt_;

	Cell_() : type_(Type_::EMPTY) {}
	Cell_(bool b) : type_(Type_::BOOLEAN), b_(b) {}
	Cell_(double d) : type_(Type_::NUMBER), d_(d) {}
	Cell_(const Date_& dt) : type_(Type_::DATE), dt_(dt, 0) {}
	Cell_(const DateTime_& dt) : type_(Type_::DATETIME), dt_(dt) {}
	Cell_(const String_& s) : type_(Type_::STRING), s_(s) {}
	Cell_(const char* s) : type_(Type_::STRING), s_(s)	{}	// otherwise pointer can be converted to boolean
	template<class T_> Cell_(const T_* p) { static_assert(false); }	// unrecognized pointer type, hides ptr-to-bool conversion

	void Clear() { type_ = Type_::EMPTY; }
	Cell_& operator=(bool b) { type_ = Type_::BOOLEAN; b_ = b; return *this; }
	Cell_& operator=(int i) { type_ = Type_::NUMBER; d_ = i; return *this; }
	Cell_& operator=(double d) { type_ = Type_::NUMBER; d_ = d; return *this; }
	Cell_& operator=(const Date_& dt) { type_ = Type_::DATE; dt_ = DateTime_(dt, 0); return *this; }
	Cell_& operator=(const DateTime_& dt) { type_ = Type_::DATETIME; dt_ = dt; return *this; }
	Cell_& operator=(const String_& s) { type_ = Type_::STRING; s_ = s; return *this; }
	Cell_& operator=(const char* s) { type_ = Type_::STRING; s_ = s; return *this; }

	Cell_& operator=(const Cell_& rhs)
	{
		switch (rhs.type_)
		{
		case Type_::BOOLEAN:
			return operator=(rhs.b_);
		case Type_::NUMBER:
			return operator=(rhs.d_);
		case Type_::STRING:
			return operator=(rhs.s_);
		case Type_::DATE:
			return operator=(rhs.dt_.Date());
		case Type_::DATETIME:
			return operator=(rhs.dt_);
		default:
			assert(rhs.type_ == Type_::EMPTY);
			Clear();
		}
		return *this;
	}
};

bool operator==(const Cell_& lhs, const String_& rhs);
inline bool operator==(const String_& lhs, const Cell_& rhs) { return rhs == lhs; }

namespace Cell
{
	inline bool IsEmpty(const Cell_& cell) { return cell.type_ == Type_::EMPTY || (cell.type_ == Type_::STRING && cell.s_.empty()); }

	inline bool IsString(const Cell_& src) { return src.type_ == Cell::Type_::STRING; }
	String_ OwnString(const Cell_& src);	// throws on non-string values; see also CoerceToString
	inline Cell_ FromString(const String_& src) { return Cell_(src); }

	inline bool IsDouble(const Cell_& src) { return src.type_ == Cell::Type_::NUMBER; }
	double ToDouble(const Cell_& src);	// throws on non-numeric values
	inline Cell_ FromDouble(double src) { return Cell_(src); }

	bool IsInt(const Cell_& src);
	int ToInt(const Cell_& src);	// throws on non-integer values
	inline Cell_ FromInt(int src) { return Cell_(double(src)); }

	inline bool IsBool(const Cell_& src) { return src.type_ == Cell::Type_::BOOLEAN; }
	bool ToBool(const Cell_& src);	// throws on non-boolean values
	inline Cell_ FromBool(bool src) { return Cell_(src); }

	inline bool IsDate(const Cell_& src) { return src.type_ == Cell::Type_::DATE; }
	Date_ ToDate(const Cell_& src);	// throws on non-date values
	inline Cell_ FromDate(const Date_& src) { return Cell_(src); }

	inline bool IsDateTime(const Cell_& src) { return src.type_ == Cell::Type_::DATETIME; }
	DateTime_ ToDateTime(const Cell_& src);	// throws on non-time values (including date w/o time)
	inline Cell_ FromDateTime(const DateTime_& src) { return Cell_(src); }

	Vector_<bool> ToBoolVector(const Cell_& src);	// accepts single boolean, or String_
	Cell_ FromBoolVector(const Vector_<bool>& src);
}

