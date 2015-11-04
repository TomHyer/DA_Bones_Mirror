
// risk report object

#pragma once

#include <deque>
#include <map>
#include "Matrix.h"
#include "Algorithms.h"
#include "Cell.h"
#include "Storable.h"


namespace Report
{
	struct Address_
	{
		std::map<String_, int> axes_;
		Vector_<int> locs_;
		Address_(const std::map<String_, int>& axes) : axes_(axes), locs_(axes.size()) {}
		int& operator[](const String_& axis);
	};

	struct Header_
	{
		Vector_<String_> labels_;
		Matrix_<Cell_> values_;	// a column for each label, a row for each entry
	};

	struct Axis_
	{
		String_ name_;
		int size_;
		Vector_<String_> labels_;	// will be placed in a Header_
	};
}

class Report_ : public Storable_
{
	std::map<String_, int> axes_;   // lookup location
	Vector_<int> strides_;
	std::deque<double> vals_;
	Vector_<Report::Header_> headers_;

public:
	Report_(const String_& name, const Vector_<Report::Axis_>& axes);

	void Write(Archive::Store_& dst) const override;
	Vector_<String_> Axes() const { return Keys(axes_); }
	int Size(const String_& axis) const;

	typedef Report::Address_ Address_;
	Address_ MakeAddress() const;
	double& operator[](const Address_& loc);
	const double& operator[](const Address_& loc) const;
	void SetAll(const Vector_<>& vals);

	const Report::Header_& Header(const String_& axis) const;
	void AddHeaderRow
		(const String_& axes,
		 int offset,
		 const Vector_<Cell_>& values);
};

