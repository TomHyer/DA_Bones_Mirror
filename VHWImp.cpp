
#include "Platform.h"
#include "VHWImp.h"
#include "Strict.h"

#include "Archive.h"
#include "PiecewiseConstant.h"

using std::map;
using VHWImp::Piece_;

/*IF--------------------------------------------------------------------------
storable VHW_Vol
	Volatility for VHW model
version 1
manual
&members
name is ?string
g is handle PiecewiseConstant
	The (time-dependent) volatility of the state S
H is handle PiecewiseConstant
	The (time-dependent) sensitivity of F(T) to S
-IF-------------------------------------------------------------------------*/

namespace
{
	typedef VHWImp::Vol_ VHW_Vol_;
	typedef pair<DateTime_, Piece_> kv_t;
#include "MG_VHW_Vol_v1_Write.inc"
#include "MG_VHW_Vol_v1_Read.inc"

	Storable_* VHW_Vol_v1::Reader_::Build() const
	{
		Vector_<DateTime_> knots = Unique(Concatenate(g_->knotDates_, H_->knotDates_));
		map<DateTime_, Piece_> vals;
		for (const auto& t : knots)
		{
			vals[t].g_ = PWC::F(*g_, t);
			vals[t].h_ = PWC::F(*H_, t);
		}
		return new VHWImp::Vol_(vals, name_);
	}

	// support Integrate() with a free function
	void Integrate0
		(const map<DateTime_, Piece_>& vol,
		 const DateTime_& to,
		 double* b,
		 double* var_s,
		 double* cov_d_s,
		 double* var_d)
	{
		assert(!vol.empty());
		auto pGE = vol.lower_bound(to);
		if (pGE == vol.begin())
		{
			ASSIGN(b, 0.0); 
			ASSIGN(var_s, 0.0);
			ASSIGN(cov_d_s, 0.0);
			ASSIGN(var_d, 0.0);
			return;
		}
		// if we hit a knot point, we integrate from the previous one rather than use the stored values
			// thus this function can be used for initialization
		auto pLT = Previous(vol.upper_bound(to));
		auto dt = to - pLT->first;
		const auto& sofar = pLT->second;
		const double g2 = Square(sofar.g_);
		const double bMid = sofar.B_ + 0.5 * dt * sofar.h_;
		ASSIGN(b, sofar.B_ + dt * sofar.h_);
		ASSIGN(var_s, sofar.varS_ + dt * g2);
		ASSIGN(cov_d_s, sofar.covDS_ - g2 * dt * bMid);
		ASSIGN(var_d, sofar.varD_ + g2 * dt * (Square(bMid) + Square(dt * sofar.h_) / 12.0));	// need +1/3 dt^2 h^2; bMid^2 gives only +1/4
	}

	// precompute needed integrals
	void Initialize(map<DateTime_, Piece_>* vol)
	{
		for (auto&& piece : *vol)
			Integrate0(*vol, piece.first, &piece.second.B_, &piece.second.varS_, &piece.second.covDS_, &piece.second.varD_);
	}
}

VHWImp::Vol_::Vol_
	(const map<DateTime_, Piece_>& vol,
	 const String_& name)
:
Storable_("VHWVol", name),
vol_(vol)
{
	Initialize(&vol_);
}

void VHWImp::Vol_::Integrate
	(const DateTime_& from,
	 const DateTime_& to,
	 double* b,
	 double* var_s,
	 double* cov_d_s,
	 double* var_d)
const
{
	double b1, b2, vs1, vs2, c1, c2, vd1, vd2;
	// postponed -- optimize this by not computing unnecessary integrals from zero
	Integrate0(vol_, from, &b1, &vs1, &c1, &vd1);
	Integrate0(vol_, to, &b2, &vs2, &c2, &vd2);
	ASSIGN(b, b2 - b1);
	ASSIGN(var_s, vs2 - vs1);
	auto c = c2 - c1 - b1 * (vs2 - vs1);
	ASSIGN(cov_d_s, c);
	ASSIGN(var_d, vd2 - vd1 - Square(b1) * (vs2 - vs1) - 2 * b1 * c);
}

void VHWImp::Vol_::Write(Archive::Store_& dst) const
{
   VHW_Vol_v1::XWrite(dst, name_, G(), H());
}

Handle_<PiecewiseConstant_> VHWImp::Vol_::G() const
{
	return new PiecewiseConstant_
			(Apply([](const kv_t& kv){ return kv.first; }, vol_),
			 Apply([](const kv_t& kv){ return kv.second.g_; }, vol_));
}

Handle_<PiecewiseConstant_> VHWImp::Vol_::H() const
{
	return new PiecewiseConstant_
			(Apply([](const kv_t& kv){ return kv.first; }, vol_),
			 Apply([](const kv_t& kv){ return kv.second.h_; }, vol_));
}


