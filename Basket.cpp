
#include "Platform.h"
#include "Basket.h"
#include "Strict.h"

#include "Vectors.h"
#include "SquareMatrix.h"

namespace
{
	void LognormalMoments
		(const Vector_<>& wf,	// weighted forwards
		const SquareMatrix_<>& ec,	// exponentiated covariance
		double* m1,
		double* m2,
		double* m3)
	{
		const int n = wf.size();
		*m1 = *m2 = *m3 = 0.0;
		double m22 = 0.0;
		double m32 = 0.0, m33 = 0.0;
		for (int ii = 0; ii < n; ++ii)
		{
			if (IsZero(wf[ii]))
				continue;
			const double& si = wf[ii];
			*m1 += si;
			*m2 += Square(si) * ec(ii, ii);
			*m3 += Cube(si * ec(ii, ii));
			for (int jj = 0; jj < ii; ++jj)
			{
				if (IsZero(wf[jj]))
					continue;
				const double& sj = wf[jj];
				m22 += si * sj * ec(jj, ii);
				m32 += si * sj * Square(ec(jj, ii)) * (si * ec(ii, ii) + sj * ec(jj, jj));
				for (int kk = jj + 1; kk < ii; ++kk)
					m33 += si * sj * wf[kk] * ec(jj, ii) * ec(kk, ii) * ec(jj, kk);
			}
		}
		*m2 += 2.0 * m22;
		*m3 += 3.0 * m32 + 6.0 * m33;
	}
}

