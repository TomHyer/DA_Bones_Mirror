
#include "Platform.h"
#include "Host.h"
#include <windows.h>
#include "Strict.h"

#include "Algorithms.h"

void Host::LocalTime
	(int* year,
	int* month,
	int* day,
	int* hour,
	int* minute,
	int* second)
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	ASSIGN(year, lt.wYear);
	ASSIGN(month, lt.wMonth);
	ASSIGN(day, lt.wDay);
	ASSIGN(hour, lt.wHour);
	ASSIGN(minute, lt.wMinute);
	ASSIGN(second, lt.wSecond);
}

