
// Monte Carlo support for component-based models

#pragma once

class SubStepper_ : noncopyable
{
public:
	virtual ~SubStepper_();
	virtual MonteCarlo::Workspace_* NewWorkspace
		(const MonteCarlo::record_t&)
	const = 0;
	virtual int NumGaussians() const = 0;
	virtual void Step
		(Vector_<>::const_iterator iid,
		 Vector_<>::iterator state,
		 MonteCarlo::Workspace_* work,
		 ComponentDf_* dfs,
		 ComponentQVols_* quantos,
		 Random_* extras,
		 Vector_<Handle_<DefaultEvent_> >* defaults)
	const = 0;
	virtual void ApplyDf(const ComponentDf_& dfs) const {}
};
