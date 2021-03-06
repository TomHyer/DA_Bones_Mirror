
#include "Platform.h"
#include "Globals.h"
#include <map>
#include <mutex>
#include "Strict.h"

#include "Host.h"
#include "Cell.h"
#include "Matrix.h"
#include "MatrixUtils.h"
#include "CellUtils.h"

Global::Store_::~Store_()
{	}

namespace
{
	// this needs to be a Meyers singleton, not a file-scope static, because we will initialize it with a file-scope static
	std::unique_ptr<Global::Store_>& XTheDateStore()
	{
      RETURN_STATIC(std::unique_ptr<Global::Store_>);
	}

	static std::mutex TheStoreMutex;	// locks all the stores there are
#define LOCK_STORES std::lock_guard<std::mutex> l(TheStoreMutex)

	// utility functions to store a date by name
	Date_ GetGlobalDate(const String_& which)
	{
		LOCK_STORES;
		const Matrix_<Cell_>& stored = Global::TheDateStore().Get(which);
		if (stored.Empty() || Cell::IsEmpty(stored(0, 0)))
		{
			// no global date set; initialize to system date
			int yy, mm, dd;
			Host::LocalTime(&yy, &mm, &dd);
			Date_ retval(yy, mm, dd);
			Global::TheDateStore().Set(which, Matrix::M1x1(Cell_(retval)));
			return retval;
		}
		return Cell::ToDate(stored(0, 0));
	}

	// names for the global dates (these will be visible in the repository, so make them comprehensible)
	static const String_ ACCOUNTING("AccountingDate");
}

Global::Store_& Global::TheDateStore()
{
	return *XTheDateStore();
}

void Global::SetTheDateStore(Global::Store_* orphan)
{
	XTheDateStore().reset(orphan);
}

Date_ Global::Dates_::AccountingDate() const
{
	return GetGlobalDate(ACCOUNTING);
}

void XGLOBAL::SetAccountingDate(const Date_& when)
{
	Global::TheDateStore().Set(ACCOUNTING, Matrix::M1x1(Cell_(when)));
}

XGLOBAL::ScopedOverride_<Date_> XGLOBAL::SetAccountingDateInScope(const Date_& dt)
{
	ScopedOverride_<Date_> retval(SetAccountingDate, GetGlobalDate(ACCOUNTING));
	SetAccountingDate(dt);
	return retval;
}

//----------------------------------------------------------------------------

// fixings history store looks very much like date store

namespace
{
	// this needs to be a Meyers singleton, not a file-scope static, because we will initialize it with a file-scope static
	std::unique_ptr<Global::Store_>& XTheFixingsStore()
	{
		RETURN_STATIC(std::unique_ptr<Global::Store_>);
	}

	static const String_ FIX_PREFIX("FixingsFor:");
}	// leave local

FixHistory_ Global::Fixings_::History(const String_& index)
{
	LOCK_STORES;
	const Matrix_<Cell_>& stored = Global::TheFixingsStore().Get(FIX_PREFIX + index);
	FixHistory_ retval;
	if (stored.Empty())
		return retval;
	assert(stored.Cols() == 2);
	assert(AllOf(stored.Col(0), Cell::TypeCheck_().DateTime()));
	assert(AllOf(stored.Col(0), Cell::TypeCheck_().Number()));
	const int n = stored.Rows();
	retval.vals_.Resize(n);
	for (int ii = 0; ii < n; ++ii)
		retval.vals_[ii] = make_pair(Cell::ToDateTime(stored(ii, 0)), Cell::ToDouble(stored(ii, 1)));
	return retval;
}

int XGLOBAL::StoreFixings
	(const String_& index,
	 const FixHistory_& fixings,
	 bool append)
{
	std::map<DateTime_, double> all;
	if (append)
	{
		const FixHistory_& old = Global::Fixings_().History(index);
		for (const auto& d_f : old.vals_)
			all[d_f.first] = d_f.second;
	}
	// now add new fixings, overwriting the old
	for (const auto& d_f : fixings.vals_)
		all[d_f.first] = d_f.second;
	// write results directly into the table for storage
	Matrix_<Cell_> storeMe(static_cast<int>(all.size()), 2);
	int ii = 0;
	for (const auto& d_f : all)
	{
		storeMe(ii, 0) = d_f.first;
		storeMe(ii, 1) = d_f.second;
		++ii;
	}	// thus stored fixings will always be in chronological order
	Global::TheFixingsStore().Set(FIX_PREFIX + index, storeMe);
	return storeMe.Rows();
}

Global::Store_& Global::TheFixingsStore()
{
	return *XTheFixingsStore();
}

void Global::SetTheFixingsStore(Global::Store_* orphan)
{
	XTheFixingsStore().reset(orphan);
}
