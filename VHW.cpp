
#include "Platform.h"
#include "VHW.h"
#include "Strict.h"

#include "VHWImp.h"
#include "YC.h"
#include "Model.h"
#include "SDEImp.h"
#include "Step.h"
#include "CurrencyData.h"
#include "SquareMatrix.h"
#include "IndexIr.h"
#include "PeriodLength.h"
#include "DayBasis.h"
#include "DateIncrement.h"

using namespace VHWImp;

namespace
{
/*IF--------------------------------------------------------------------------
storable VHW
	Vasicek-Hull-White model
&members
name is ?string
yc is handle YieldCurve
	Underlying discount and libor curve
vol is handle VHWImp::Vol
	Volatility structure
-IF-------------------------------------------------------------------------*/
#include "MG_VHW_Write.inc"

	class VHW_ : public Model_, public SDEImp_
	{
		Handle_<YieldCurve_> yc_;
		Handle_<Vol_> vol_;
      Handle_<SDE_> self_; // a duplicate of *this

		void Write(Archive::Store_& dst) const override
		{
         VHW::XWrite(dst, name_, yc_, vol_);
		}
	public:
		VHW_(const String_& name,
			const Handle_<YieldCurve_>& yc,
			const Vol_& vol);

		Handle_<YieldCurve_> YieldCurve
			(const Ccy_& ccy)
		const override
		{
			REQUIRE(ccy == yc_->ccy_, "No curve for '" + String_(ccy.String()) + "'");
			return yc_;
		}
      DateTime_ VolStart() const override { return vol_->VolStart(); }
		Handle_<SDE_> ForTrade
			(_ENV, const Underlying_& trade)
			const override
		{
			// ideally we would return 'this', but have to ensure the memory management is right (requires intrusive handles so an object can put itself in a handle)
         //if (!self_)
         //   self_.reset(new VHW_(String_(), yc_, vol_));
			return self_;
		}	

		SDEImp::UpdateOne_* NewUpdateOne
			(_ENV, RequestAtTime_& t,
			const Index_& index)
		const override;

		VHW_* Mutant_Model
			(const String_* new_name,
			 const Slide_* slide)
		const override;
	};
//#include "MG_VHW_Read.inc" // can't build a VHW_, implementation is incomplete

	// interface to numerical solvers
	class VHWStep_ : public ModelStepper_
	{
		double sqrtDt_;
		double muS_, sigmaS_;   // de-annualized!
		double a_, bMinus_, bPlus_;
	public:
		MonteCarlo::Workspace_* NewWorkspace
			(const MonteCarlo::record_t&)
			const override
		{
			return 0;
		}
		int NumGaussians() const override { return 1; }
		void Step
			(Vector_<>::const_iterator iid,
			Vector_<>* state,
			MonteCarlo::Workspace_*,
			Random_*,
			double* rolling_df,
			Vector_<Handle_<DefaultEvent_> >*)
		const override
		{
			const double sMinus = state->front();
			state->front() += muS_ + sigmaS_ * *iid;
			*rolling_df *= exp(a_ - bMinus_ * sMinus - bPlus_ * state->front());
		}
		PDE::ScalarCoeff_* DiscountCoeff() const override
		{
			struct Mine_ : PDE::ScalarCoeff_
			{
				x_dep_t xDep_;
				double c0_, c1_;
				Mine_(double c0, double c1) : c0_(c0), c1_(c1)
				{
					xDep_.set(0);
				}
				void Value(const Vector_<>& x,
					double* value)
				const override
				{
					assert(x.size() == 1);   // just S
					*value = c0_ + c1_ * x[0];
				}
				x_dep_t XDependence() const override { return xDep_; }
			};
			const double dt = Square(sqrtDt_);
			return new Mine_(a_ / dt, (bPlus_ + bMinus_) / dt);
		}
		PDE::VectorCoeff_* AdvectionCoeff() const override
		{
			return PDE::NewConstCoeff(Vector::V1(0.0));
		}
		PDE::MatrixCoeff_* DiffusionCoeff() const override
		{
			return PDE::NewConstCoeff(SquareMatrix::M1x1(0.5 * Square(sigmaS_ / sqrtDt_)));
		}
	};

	struct VHWAccumulator_ : StepAccumulator_
	{
		DateTime_ volStart_;
		const VHWImp::Vol_& vols_;

		VHWAccumulator_(const DateTime_& vol_start, const VHWImp::Vol_& vols) : volStart_(vol_start), vols_(vols) {}
		Vector_<pair<double, double> > Envelope
			(const DateTime_& t,
			double num_sigma)
		const override
		{
			double varS, eS;
			vols_.Integrate(volStart_, t, 0, &varS, &eS, 0);
			const double halfWidth = sqrt(varS) * num_sigma;
			return Vector::V1(make_pair(eS - halfWidth, eS + halfWidth));
		}
	};

	// interface to payouts

	SDEImp::UpdateOne_* NewLiborUpdater
		(_ENV, const YieldCurve_& yc,
		const VHWImp::Vol_& vol,
		const DateTime_& event_time,
		const Date_& start_date,
		const PeriodLength_& tenor)
	{
		struct Mine_ : SDEImp::UpdateOne_
		{
			double A_, B_, dct_;
			Mine_(double A, double B, double dct) : A_(A), B_(B), dct_(dct) {}
			double operator()
				(const Vector_<>::const_iterator& state,
				 const UpdateToken_& prior) 
			const override
			{
				return (A_ * exp(B_ * state[0]) - 1.0) / dct_;
			}
		};

		const double F = yc.FwdLibor(tenor, start_date);
		const Date_ maturity = Date::NominalMaturity(start_date, tenor, yc.ccy_);
		const double dct = Ccy::Conventions::LiborDayBasis()(yc.ccy_)(start_date, maturity, nullptr);
		double B, ES, varS;
		vol.Integrate(DateTime_(start_date), DateTime_(maturity), &B, nullptr);
		vol.Integrate(vol.VolStart(), event_time, nullptr, &varS, &ES);
		const double A = (1 + dct * F) * exp(-B * (ES + 0.5 * B * varS));
		return new Mine_(A, B, dct);
	}

	SDEImp::UpdateOne_* NewDfUpdater
		(const YieldCurve_& yc,
		const VHWImp::Vol_& vol,
		const DateTime_& event_time,
		const Index::DF_& index)
	{
		struct Mine_ : SDEImp::UpdateOne_
		{
			double A_, B_;
			Mine_(double A, double B) : A_(A), B_(B) {}
			double operator()
				(const Vector_<>::const_iterator& state,
				 const UpdateToken_& prior) 
			const
			{
				return A_ * exp(B_ * *state);
			}
		};

		const Date_ from = index.StartDate(event_time), to = index.Maturity(event_time);
		const double Z = yc.Discount(index.collateral_)(from, to);
		double B, ES, varS;
		vol.Integrate(DateTime_(from), DateTime_(to), &B, nullptr);
		vol.Integrate(vol.VolStart(), event_time, nullptr, &varS, &ES);
		const double A = Z * exp(-B * (ES + 0.5 * B * varS));
		return new Mine_(A, B);
	}

	SDEImp::UpdateOne_* VHW_::NewUpdateOne
		(_ENV, RequestAtTime_& t,
		 const Index_& index)
	const
	{
		if (auto ir = dynamic_cast<const Index::IRForward_*>(&index))
		{
			REQUIRE(ir->ccy_ == yc_->ccy_, "Non-domestic rate index requested");
			const Date_ start = ir->StartDate(t.t_);
			if (auto il = dynamic_cast<const Index::Libor_*>(ir))
				return NewLiborUpdater(_env, *yc_, *vol_, t.t_, start, il->tenor_.Period());
			if (auto is = dynamic_cast<const Index::Swap_*>(ir))
				return SDEImp::NewSwapUpdater(t, *is);
			THROW("Unrecognized interest rate index");
		}
		if (auto df = dynamic_cast<const Index::DF_*>(&index))
		{
			return NewDfUpdater(*yc_, *vol_, t.t_, *df);
		}
		THROW("VHW model can't simulate non-IR indices");
	}
}	// leave local

Model_* ::VHW::NewHoLee
	(const String_& name,
	 const Handle_<YieldCurve_>& yc,
	 const DateTime_& vol_start,
	 double vol)
{
	// create a constant vol
	std::map<DateTime_, VHWImp::Piece_> volMap;
	auto& piece = volMap[vol_start];
	piece.g_ = vol;
	piece.h_ = 1.0;
//	return new VHW_(name, yc, new VHWImp::Vol_(volMap));
	return nullptr;
}