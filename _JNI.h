
// supporting includes and translators for JNI interface wrapper

#pragma once

#include <jni.h>
#include "Dictionary.h"
#include "Optionals.h"
#include "Exceptions.h"

class Storable_;

namespace JNI
{
	double ToDouble(JNIEnv* jenv, const jdouble src);
	boost::optional<double> ToDouble(JNIEnv* jenv, const jdouble src, bool);	// use overloading to know the return type
	int ToInt(JNIEnv* jenv, const jint src);
	boost::optional<int> ToInt(JNIEnv* jenv, const jint src, bool);
	bool ToBool(JNIEnv* jenv, const jboolean src);
	boost::optional<bool> ToBool(JNIEnv* jenv, const jboolean src, bool);
	String_ ToString(JNIEnv* jenv, const jstring src, bool optional = false);
	Date_ ToDate(JNIEnv* jenv, const jobject src, bool optional = false);
	DateTime_ ToDateTime(JNIEnv* jenv, const jobject src, bool optional = false);
	Cell_ ToCell(JNIEnv* jenv, const jobject src, bool optional = false);
	Dictionary_ ToDictionary(JNIEnv* jenv, const jobject src, bool optional = false);
	Handle_<Storable_> ToHandleBase(JNIEnv* jenv, const jobject src, bool optional = false);
	Vector_<> ToDoubleVector(JNIEnv* jenv, const jdoubleArray src, bool optional = false);
	Vector_<int> ToIntVector(JNIEnv* jenv, const jintArray src, bool optional = false);
	Vector_<bool> ToBoolVector(JNIEnv* jenv, const jbooleanArray src, bool optional = false);
	Vector_<String_> ToStringVector(JNIEnv* jenv, const jobjectArray src, bool optional = false);
	Vector_<Date_> ToDateVector(JNIEnv* jenv, const jobjectArray src, bool optional = false);
	Vector_<Cell_> ToCellVector(JNIEnv* jenv, const jobjectArray src, bool optional = false);
	Vector_<Handle_<Storable_> > ToHandleBaseVector(JNIEnv* jenv, const jobjectArray src, bool optional = false);
	Vector_<Dictionary_> ToDictionaryVector(JNIEnv* jenv, const jobjectArray src);
	Matrix_<double> ToDoubleMatrix(JNIEnv* jenv, const jobjectArray src, bool optional = false);
	Matrix_<int> ToIntMatrix(JNIEnv* jenv, const jobjectArray src, bool optional = false);
	// Matrix of bool is presently not supported
	Matrix_<String_> ToStringMatrix(JNIEnv* jenv, const jobjectArray src, bool optional = false);
	Matrix_<Cell_> ToCellMatrix(JNIEnv* jenv, const jobjectArray src, bool optional = false);

	template<class T_> T_ ToEnum(JNIEnv* jenv, const jstring src) { return T_(ToString(jenv, src)); }
	template<class T_> boost::optional<T_> ToEnum(JNIEnv* jenv, const jstring src, bool optional)
	{
		boost::optional<T_> retval;
		const String_ s = ToString(jenv, src, optional);
		if (!s.empty())
			retval = T_(s);
		return retval;
	}

	template<class T_> Handle_<T_> ToHandle(JNIEnv* jenv, const jobject src, bool optional = false)
	{
		const Handle_<Storable_> base = ToHandleBase(jenv, src, optional);
		if (base.Empty() && optional)
			return Handle_<T_>();

		const Handle_<T_> retval = handle_cast<T_>(base);
		REQUIRE(!retval.Empty(), "Input object has wrong type");
		return retval;
	}
	template<class T_> Vector_<Handle_<T_> > ToHandleVector(JNIEnv* jenv, const jobjectArray src)
	{
		const Vector_<Handle_<Storable_> > base = ToHandleBaseVector(jenv, src);
		Vector_<Handle_<T_> > retval;
		for (const auto& b : base)
		{
			retval.push_back(handle_cast<T_>(b));
			REQUIRE(!retval.back().Empty(), "Input object has wrong type");
		}
		return retval;
	}
	template<class T_> T_ ToSettings(JNIEnv* jenv, const jobject src, bool optional = false)
	{
		return T_(ToDictionary(jenv, src));
	}

	// enhanced dictionary which also contains handles
	// this implementation is kind of cheesy, because I have not decided whether all dictionaries should be able to contain handles
	struct HDict_
	{
		Dictionary_ atoms_;
		std::map<String_, Handle_<Storable_>> objects_;
	};
	Vector_<HDict_> ToHDictionaryVector(JNIEnv* jenv, const jobjectArray src);
	template<class T_> Vector_<T_> ToRecordVector(JNIEnv* jenv, const jobjectArray src, bool optional = false)
	{
		Vector_<T_> retval;
		for (const auto& d : ToHDictionaryVector(jenv, src))
			retval.emplace_back(d.atoms_, d.objects_);
		return retval;
	}

	// outputs -- we always give Java a copy of our native object
	inline jint CopyOut(JNIEnv* jenv, int src) { return src; }
	inline jdouble CopyOut(JNIEnv* jenv, double src) { return src; }
	jstring CopyOut(JNIEnv* jenv, const String_& src);
	jobject CopyOut(JNIEnv* jenv, const Date_& src);
	jobject CopyOut(JNIEnv* jenv, const DateTime_& src);
	jobject CopyOut(JNIEnv* jenv, const Cell_& src);
	jobject CopyOutBase(JNIEnv* jenv, const Handle_<Storable_>& src);
	template<class T_> Handle_<Storable_> ToStorable(const Handle_<T_>& src)
	{
		auto retval = handle_cast<Storable_>(src);
		REQUIRE(retval || !src, "Output object is not storable");
		return retval;
	}
	template<class T_> jobject CopyOut(JNIEnv* jenv, const Handle_<T_>& src)
	{
		return CopyOutBase(jenv, ToStorable(src));
	}

	jdoubleArray CopyOut(JNIEnv* jenv, const Vector_<>& src);
	jintArray CopyOut(JNIEnv* jenv, const Vector_<int>& src);
	jobjectArray CopyOut(JNIEnv* jenv, const Vector_<String_>& src);
	jobjectArray CopyOut(JNIEnv* jenv, const Vector_<Cell_>& src);
	jobjectArray CopyOutBase(JNIEnv* jenv, const Vector_<Handle_<Storable_>>& src);
	template<class T_> jobjectArray CopyOut(JNIEnv* jenv, const Vector_<Handle_<T_>>& src)
	{
		return CopyOutBase(jenv, Apply(ToStorable<T_>, src));
	}

	jobjectArray CopyOut(JNIEnv* jenv, const Matrix_<>& src);
	jobjectArray CopyOut(JNIEnv* jenv, const Matrix_<String_>& src);
	jobjectArray CopyOut(JNIEnv* jenv, const Matrix_<Cell_>& src);

	// errors
	jobject Error(JNIEnv* jenv, const char* msg, const char* arg_name);
}

