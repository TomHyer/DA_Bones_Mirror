
// rectangular arrays of arbitrary dimension; compare with numpy arrays

#pragma once

#include "Vectors.h"
#include "Exceptions.h"

namespace ArrayN
{
	Vector_<int> Strides(const Vector_<int>& sizes);	// create strides so back()==1
	Vector_<pair<int, int>> Moves(const Vector_<int>& old_sizes, const Vector_<int>& new_sizes);
}

// can't use boost::multi_array because we need to Swap with a vector
template<class E_> class ArrayN_
{
	Vector_<int> sizes_;
	Vector_<int> strides_;
	Vector_<E_> vals_;	// member ordering used in constructor
public:
	ArrayN_(const Vector_<int>& sizes, const E_& fill = 0)
		:
	sizes_(sizes),
	strides_(ArrayN::Strides(sizes)),
	vals_(strides_[0] * sizes[0], fill)
	{	}

	const E_& operator[](const Vector_<int>& where) const { return vals_[InnerProduct(where, strides_)]; }
	E_& operator[](const Vector_<int>& where) { return vals_[InnerProduct(where, strides_)]; }

   inline const Vector_<int>& Sizes() const { return sizes_; }
   inline bool Empty() const { return vals_.empty(); }
	inline void Fill(double val) { vals_.Fill(val); }
	inline void operator*=(double scale) { vals_ *= scale; }

	void Swap(ArrayN_<E_>* other)
	{
		sizes_.Swap(other->sizes_);
		strides_.Swap(other->strides_);
		vals_.Swap(other->vals_);
	}
	// also allow Swap with a vector, iff we are effectively one-dimensional
	void Swap(Vector_<E_>* other)
	{
		auto pm = MaxElement(sizes_);
		REQUIRE(*pm == vals_.size(), "Can't swap a vector with a multi-dimensional array");
		vals_.Swap(other);
		*pm = vals_.size();
		strides_ = Strides(sizes_);
	}
	// Resize() requires computation of element mapping; take it out-of-line
	void Resize(const Vector_<int>& new_sizes)
	{
		const Vector_<pair<int, int> >& moves = ArrayN::Moves(sizes_, new_sizes);
		Vector_<E_> newVals(sizes_[0] * strides_[0], E_());
		sizes_ = new_sizes;
		strides_ = ArrayN::Strides(sizes_);
		for (const auto& move : moves)
			newVals[move.second] = vals_[move.first];
		vals_.Swap(&newVals);
	}
	// allow enough access for Cube_
protected:
   struct XLoc_
	{
		int offset_;
		int sofar_;
		const Vector_<int>& strides_;
		XLoc_(const Vector_<int>& strides) : offset_(0), sofar_(0), strides_(strides) {}
		XLoc_& operator()(int i_x) { sofar_ += i_x * strides_[offset_++]; return *this; }
      int Offset() const { assert(offset_ == strides_.size()); return sofar_; }
	};
	XLoc_ Goto() const { return XLoc_(strides_); }
	const E_& At(const XLoc_& loc) const { return vals_[loc.Offset()]; }
	E_& At(const XLoc_& loc) { return vals_[loc.Offset()]; }
};

class Cube_ : public ArrayN_<double>
{
public:
	Cube_();
	Cube_(int size_i, int size_j, int size_k);
	// support lookups without constructing a temporary vector
	const double& operator()(int ii, int jj, int kk) const
	{
		return At(Goto()(ii)(jj)(kk));
	}
	double& operator()(int ii, int jj, int kk)
	{
		return At(Goto()(ii)(jj)(kk));
	}
	// allow access to slices (last dimension)
	inline double* SliceBegin(int ii, int jj) { return &operator()(ii, jj, 0); }
	inline const double* SliceBegin(int ii, int jj) const { return &operator()(ii, jj, 0); }
	inline const double* SliceEnd(int ii, int jj) const { return SliceBegin(ii, jj) + Goto()(0)(1)(0).Offset(); }	

	inline int SizeI() const { return Sizes()[0]; }
	inline int SizeJ() const { return Sizes()[1]; }
	inline int SizeK() const { return Sizes()[2]; }
	void Resize(int size_i, int size_j, int size_k);
};

