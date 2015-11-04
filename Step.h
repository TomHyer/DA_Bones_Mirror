
// model stepper, for transitions between event times in PDE or MC pricing

#pragma once

#include "MCPath.h"
#include "PDE.h"
#include "Random.h"
#include "DefaultModel.h"

class ModelStepper_ : noncopyable
{
public:
	virtual ~ModelStepper_();

	// PDE interface
	virtual PDE::ScalarCoeff_* DiscountCoeff() const;
	virtual PDE::VectorCoeff_* AdvectionCoeff() const;
	virtual PDE::MatrixCoeff_* DiffusionCoeff() const;

	// MC interface
	virtual MonteCarlo::Workspace_* NewWorkspace
		(const MonteCarlo::record_t& paths_record)
	const = 0;
	virtual int NumGaussians() const = 0;
	virtual void Step
		(Vector_<>::const_iterator iid_gaussian_begin,
		 Vector_<>* state,
		 MonteCarlo::Workspace_* work,
		 Random_* more_randoms,
		 double* rolling_df,
		 Vector_<Handle_<DefaultEvent_> >* defaults)
	const = 0;
};

class StepAccumulator_ : noncopyable
{
public:
	virtual ~StepAccumulator_();

	virtual MonteCarlo::PathsRecord_* NewPathsRecord
		(int num_paths,
		 const MonteCarlo::record_t& base_record)
	const
	{
		return 0;
	}

	virtual Vector_<pair<double, double> > Envelope
		(const DateTime_& t,
		 double num_sigma)
	const = 0;
};
