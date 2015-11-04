
// internals of Vasicek-Hull-White model

#pragma once

#include <map>
#include "Storable.h"
#include "DateTime.h"

struct PiecewiseConstant_;

namespace VHWImp
{
	struct Piece_
	{
		double g_;
		double h_;
		// integrals to start:
		double B_;
		double varS_;
		double covDS_;   // will be negative
		double varD_;
	};

	class Vol_ : public Storable_
	{
		std::map<DateTime_, Piece_> vol_;
		void Write(Archive::Store_& dst) const override;
	public:
		Vol_(const std::map<DateTime_, Piece_>& vol,
			const String_& name = String_());

		void Integrate
			(const DateTime_& from,
			const DateTime_& to,
			double* b,
			double* var_s,
			double* cov_d_s = nullptr,
			double* var_d = nullptr)
			const;

		// allow query of data -- these functions allocate and populate the return structure, thus are not very efficient
		Handle_<PiecewiseConstant_> G() const;
		Handle_<PiecewiseConstant_> H() const;
		DateTime_ VolStart() const{ return vol_.begin()->first; }
	};

	Vol_* NewVol
		(const DateTime_& vol_start,
		const PiecewiseConstant_& g,
		const PiecewiseConstant_& h);
}
