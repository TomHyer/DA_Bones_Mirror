
// utility functions with no direct relation to derivatives
	// these can be very useful in tool-poor environments, e.g., Excel

#include "__Platform.h"

#include <chrono>
#include "CellUtils.h"
#include "MatrixUtils.h"

namespace
{
/*IF--------------------------------------------------------------------------
public Repeat
	Repeats a cell or array
&inputs
base is cell[][]
	The content to repeat
n_down is integer
	&$ > 0
	The number of times to repeat the content, stacked vertically
&optional
n_across is integer (1)
	&$ > 0
	The number of times to repeat the vertical stack, stacked horizontally
&outputs
repeated is cell[][]
	A repeated array
-IF-------------------------------------------------------------------------*/

	void Repeat
		(const Matrix_<Cell_>& base,
		 int n_down,
		 int n_across,
		 Matrix_<Cell_>* dst)
	{
		dst->Resize(base.Rows() * n_down, base.Cols() * n_across);
		for (int ir = 0; ir < dst->Rows(); ++ir)
		{
			auto src = base.Row(ir % base.Rows());
			for (int ia = 0; ia < n_across; ++ia)
				copy(src.begin(), src.end(), dst->Row(ir).begin() + ia * base.Cols());
		}
	}

/*IF--------------------------------------------------------------------------
public Sort_Keys
	Sorts the keys [0, n) so that x[key[i]] is in ascending order, for some input array x of size n
&inputs
x is cell[]
	The array to sort
&outputs
keys is integer[]
	Keys which order x
-IF-------------------------------------------------------------------------*/

	void Sort_Keys
		(const Vector_<Cell_>& x,
		 Vector_<int>* keys)
	{
		struct Compare_
		{
			const Vector_<Cell_>& x_;
			Compare_(const Vector_<Cell_>& x) : x_(x) {}
			bool operator()(int i, int j) const { return operator()(x_[i], x_[j]); }
			bool operator()(const Cell_& lhs, const Cell_& rhs) const
			{
				if (Cell::IsEmpty(lhs) || Cell::IsEmpty(rhs))
					return !Cell::IsEmpty(lhs);	// sort empties to the back
				if (Cell::IsString(lhs) && Cell::IsString(rhs))
					return Cell::OwnString(lhs) < Cell::OwnString(rhs);
				if (Cell::IsString(lhs) || Cell::IsString(rhs))
					return Cell::IsString(lhs);	// sort strings to the front
				return CoerceNumeric(lhs) < CoerceNumeric(rhs);
			}
			double CoerceNumeric(const Cell_& c) const
			{
				if (Cell::IsDouble(c))
					return Cell::ToDouble(c);
				if (Cell::IsDate(c))
					return NumericValueOf(Cell::ToDate(c));
				if (Cell::IsDateTime(c))
					return NumericValueOf(Cell::ToDateTime(c));
				if (Cell::IsBool(c))
					return Cell::ToBool(c) ? 1.0 : 0.0;
				assert(!"Unreachable -- bad cell type");
				return -1.0;
			}
		};
		*keys = Vector::UpTo(x.size());
		Sort(keys, Compare_(x));
	}

/*IF--------------------------------------------------------------------------
public Unstack
	Converts a vector of m*n elements into a m-by-n array
&inputs
x is cell[]
	The vector to unstack
n_cols is integer
	&$ > 0
	&x.size() % $ == 0\Size of x must be an integer multiple of $
	The number of columns to produce
&outputs
a is cell[][]
	Such that the first row of a contains the first n_cols elements of x,
	the second row contains the next n_cols elements, etc.
-IF-------------------------------------------------------------------------*/

	void Unstack
		(const Vector_<Cell_>& x,
		 int n_cols,
		 Matrix_<Cell_>* a)
	{
		const int nRows = x.size() / n_cols;
		a->Resize(nRows, n_cols);
		for (int ii = 0; ii < nRows; ++ii)
			std::copy(&x[n_cols * ii], &x[n_cols * (ii + 1)], a->Row(ii).begin());
	}

/*IF--------------------------------------------------------------------------
public Slice
	Gets an element or subset of an array
&inputs
a is cell[][]+
	The array into which to index
&optional
rows is cell[]
	A number of the (0-offset) row to take, or a string describing which rows
	to take.  The syntax is that of Python's array indexing.  Multiple entries
	can be placed in separate cells, or separated by commas.  If empty, all rows
	will be retained.  
cols is cell[]
	As for rows, but takes a subset of the columns.  If empty, all columns
	will be retained.
&outputs
slice is cell[][]
	The elements of a indicated by the input rows and cols
-IF-------------------------------------------------------------------------*/

	Vector_<int> Keep(const String_& src, int size)
	{
		NOTICE(src);
		auto colon = src.find(':');
		if (colon == String_::npos)
		{
			int loc = String::ToInt(src);
			return Vector::V1(loc < 0 ? size - loc : loc);
		}
		else
		{
			int start = colon == 0 ? 0 : String::ToInt(src.substr(0, colon));
			int stop = colon == src.size() - 1 ? size : String::ToInt(src.substr(colon + 1));
			if (start < 0)
				start += size;
			if (stop < 0)
				stop += size;
			REQUIRE(stop > start, "Invalid range");
			return Apply([&](int ii){return ii + start; }, Vector::UpTo(stop - start));
		}
	}

	Vector_<int> Keep(const Cell_& src, int size)
	{
		if (Cell::IsEmpty(src))
			return Vector_<int>();
		if (Cell::IsInt(src))
			return Vector::V1(Cell::ToInt(src));
		if (Cell::IsString(src))
		{
			auto parts = String::Split(Cell::OwnString(src), ',', false);
			Vector_<int> retval;
			for (const auto& p : parts)
				Append(&retval, Keep(p, size));
			return retval;
		}
		THROW("Invalid cell type for indexing");
	}

	Vector_<int> Keep(const Vector_<Cell_>& src, int size)
	{
		if (src.empty())
			return Vector::UpTo(size);
		Vector_<int> retval;
		for (const auto& c : src)
			Append(&retval, Keep(c, size));
		return retval;
	}

	void Slice
		(const Matrix_<Cell_>& a,
		 const Vector_<Cell_>& rows,
		 const Vector_<Cell_>& cols,
		 Matrix_<Cell_>* slice)
	{
		Vector_<int> kr = Keep(rows, a.Rows());
		Vector_<int> kc = Keep(cols, a.Cols());
		REQUIRE(!kr.empty() && !kc.empty(), "Slice is empty");
		REQUIRE(*MinElement(kr) >= 0 && *MaxElement(kr) < a.Rows(), "Row index out of range");
		REQUIRE(*MinElement(kc) >= 0 && *MaxElement(kc) < a.Cols(), "Column index out of range");
		slice->Resize(kr.size(), kc.size());
		for (int ir = 0; ir < kr.size(); ++ir)
			for (int ic = 0; ic < kc.size(); ++ic)
				(*slice)(ir, ic) = a(kr[ir], kc[ic]);
	}

/*IF--------------------------------------------------------------------------
public CoerceToString
	Converts all atomic inputs to string representations
&inputs
a is cell[][]
	The array of inputs of any (non-handle) type
&outputs
s is string[][]
	The string representation of each cell in a
-IF-------------------------------------------------------------------------*/

	void CoerceToString
		(const Matrix_<Cell_>& a,
		 Matrix_<String_>* s)
	{
		s->Resize(a.Rows(), a.Cols());
		for (int ir = 0; ir < a.Rows(); ++ir)
			std::transform(a.Row(ir).begin(), a.Row(ir).end(), s->Row(ir).begin(), Cell::CoerceToString);
	}

/*IF--------------------------------------------------------------------------
public Join
	Combines an array of strings into a single entry
&inputs
a is string[][]
	The array of strings
&optional
col_separator is string
	&a.Cols() == 1 || !$.empty()\$ can't be empty when there are multiple columns
	The separator between entries in the same row
row_separator is string
	&a.Rows() == 1 || !$.empty()\$ can't be empty when there are multiple rows
	The separator between rows 
skip_empty is boolean (false)
	If true, no separator will be supplied after an empty entry
&outputs
s is string
	A combined string of all of a
-IF-------------------------------------------------------------------------*/

	void Join
		(const Matrix_<String_>& a,
		 const String_& col_sep,
		 const String_& row_sep,
		 bool skip_empty,
		 String_* s)
	{
		for (int ir = 0; ir < a.Rows(); ++ir)
		{
			auto part = String::Accumulate(a.Row(ir), col_sep, skip_empty);
			if (!part.empty() || !skip_empty)
			{
				if (!s->empty())
					*s += row_sep;
				*s += part;
			}
		}
	}

/*IF--------------------------------------------------------------------------
public Split
	The opposite of Join
&inputs
s is string
	The string to split
&optional
col_separator is string
	&$.size() <= 1\$ can only be a single character
	Each occurrence of this separator causes a new entry to be created in the current row
row_separator is string
	&$.size() <= 1\$ can only be a single character
	&!$.empty() || !col_separator.empty()\Must supply either $ or col_separator
	Each occurrence of this separator causes a new row to be created 
skip_empty is boolean (false)
	If true, empty strings between separators are ignored
&outputs
a is string[][]
	A matrix of strings created by the separation process
-IF-------------------------------------------------------------------------*/

	void Split
		(const String_& s,
		 const String_& col_sep,
		 const String_& row_sep,
		 bool skip_empty,
		 Matrix_<String_>* a)
	{
		auto v = row_sep.empty() ? Vector::V1(s) : String::Split(s, row_sep[0], !skip_empty);
		auto splitRow = [&](const String_& src) {return String::Split(src, col_sep[0], !skip_empty); };
		auto vv = col_sep.empty() ? Vector::V1(v) : Apply(splitRow, v);
		*a = Matrix::FromVectors(vv, false, false, true);
	}

/*IF--------------------------------------------------------------------------
public Timing_Start
	Starts a (non-thread-safe) wall-clock timer
&inputs
n is cell[][]
	ignored
&outputs
x is number
	Always zero; this function is evaluated for its side effect, which is to
	set a static system-clock instance
-IF-------------------------------------------------------------------------*/

	static std::chrono::time_point<std::chrono::high_resolution_clock> TheStart;
	void Timing_Start(const Matrix_<Cell_>&, double* x)
	{
		TheStart = std::chrono::high_resolution_clock::now();
		*x = 0.0;
	}

/*IF--------------------------------------------------------------------------
public Timing_Elapsed
	Returns the number of seconds since the last call to Timing_Start
&inputs
a is cell[][]
	ignored
&outputs
dt is number
	The time since the last call to Timing_Start, in seconds
-IF-------------------------------------------------------------------------*/

	void Timing_Elapsed(const Matrix_<Cell_>&, double* dt)
	{
		auto split = std::chrono::high_resolution_clock::now();
		*dt = std::chrono::duration<double>(split - TheStart).count();
	}
}

#include "MG_Repeat_public.inc"
#include "MG_Sort_Keys_public.inc"
#include "MG_Unstack_public.inc"
#include "MG_Slice_public.inc"
#include "MG_CoerceToString_public.inc"
#include "MG_Join_public.inc"
#include "MG_Split_public.inc"
#include "MG_Timing_Start_public.inc"
#include "MG_Timing_Elapsed_public.inc"

