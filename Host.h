
// functions with system-dependent implementation

#pragma once

namespace Host
{
	void LocalTime
		(int* year,
		int* month,
		int* day,
		int* hour = nullptr,
		int* minute = nullptr,
		int* second = nullptr);
}

#define FORCE_INLINE __forceinline