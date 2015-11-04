
#include "Platform.h"
#include "LVModel.h"
#include "Strict.h"

#include "Model.h"
#include "SDEImp.h"
#include "LVSurface.h"
#include "PiecewiseConstant.h"

namespace
{
	class LVModel_ : public Model_
	{
		Handle_<YieldCurve_> yc_;
		Handle_<LVComponent_> equity_;
	public:
		LVModel_(const String_& name,
			const Handle_<YieldCurve_>& yc,
			const Handle_<LVComponent_>& eq);
		// ...
	};
}	// leave local

