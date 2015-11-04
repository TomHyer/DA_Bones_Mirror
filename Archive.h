
// abstract representation of persistence mechanism
   // Storable_ objects write to Archive::Store_ and reconstruct from Archive::View_

#pragma once

#include <map>
#include "Matrix.h"
#include "Dictionary.h"
#include "Strings.h"
// the following headers are not necessary for compilation, but this header is not useful without them
#include <functional>
#include "Storable.h"
#include "Exceptions.h"
#include "Optionals.h"

namespace Archive
{
	extern String_ VECTOR;

   namespace Utils
   {
      // specialization for storables, checks whether they are already stored
      // declared away from other utilities because it needs friend status
      void SetStorable
         (Archive::Store_& dst,
          const String_& name,
          const Storable_& value);
   }

	class Store_ : noncopyable
	{
      virtual bool StoreRef(const Storable_* object) = 0;  // returns true and reuses tag if it exists; or creates a new tag and returns false
      friend void Archive::Utils::SetStorable(Archive::Store_&, const String_&, const Storable_&);
   public:
		virtual ~Store_();
		virtual void SetType(const String_& type) = 0;
		virtual void Done() {}	// states explicitly when an object is done (to enable streaming)
		virtual Store_& Child(const String_& name) = 0;
		Store_& Element(int index);	// just converts to String_ name and gets a child

		virtual void operator=(double val) = 0;
		virtual void operator=(const String_& val) = 0;
		virtual void operator=(const Date_& val) = 0;
		virtual void operator=(const Vector_<>& val) = 0;
		virtual void operator=(const Vector_<int>& val) = 0;
		virtual void operator=(const Vector_<bool>& val) = 0;
		virtual void operator=(const Vector_<String_>& val) = 0;
		virtual void operator=(const Vector_<Date_>& val) = 0;
		virtual void operator=(const Vector_<DateTime_>& val) = 0;
		virtual void operator=(const Matrix_<>& val) = 0;
		virtual void operator=(const Matrix_<String_>& val) = 0;
		virtual void operator=(const Matrix_<Cell_>& val) = 0;
		virtual void operator=(const Dictionary_& val) = 0;
	};

   // record of what already exists
   class Built_
   {
   public:
      std::map<String_, Handle_<Storable_>> known_;
   };

   class View_;
   Handle_<Storable_> Extract
      (const Archive::View_& src,
       Built_& built);

   class View_ : noncopyable
	{
      virtual Handle_<Storable_>& Known(Archive::Built_& built) const = 0; // returns a reference within 'built'
      friend Handle_<Storable_> Archive::Extract(const Archive::View_& src, Built_& built);
   public:
		virtual ~View_();

      // query fundamental types
      virtual double AsDouble() const = 0;
      virtual int AsInt() const = 0;
	  virtual bool AsBool() const = 0;
      virtual String_ AsString() const = 0;
	  virtual Date_ AsDate() const = 0;
	  virtual Dictionary_ AsDictionary() const = 0;
      virtual Vector_<> AsDoubleVector() const = 0;
      virtual Vector_<int> AsIntVector() const = 0;
      virtual Vector_<bool> AsBoolVector() const = 0;
      virtual Vector_<String_> AsStringVector() const = 0;
      virtual Vector_<Date_> AsDateVector() const = 0;
      virtual Vector_<DateTime_> AsDateTimeVector() const = 0;
	  virtual Matrix_<> AsDoubleMatrix() const = 0;
	  virtual Matrix_<String_> AsStringMatrix() const = 0;
	  virtual Matrix_<Cell_> AsCellMatrix() const = 0;

      // query composite types
		virtual String_ Type() const = 0;	// empty for atoms
		virtual const View_& Child(const String_& name) const = 0;
		virtual bool HasChild(const String_& name) const = 0;
		const View_& Element(int index) const;
		bool HasElement(int index) const;
      // notify of unexpected children
      virtual void Unexpected(const String_& child_name) const = 0;
	};

	template<class T_ = Storable_> struct Builder_
	{
		Built_& share_;
      const char* name_;
      const char* type_;
		Builder_(Built_& share, const char* name, const char* type) : share_(share), name_(name), type_(type) {}
		Handle_<T_> operator()(const View_& src) const
		{
         NOTICE2("Child name", name_);
         Handle_<Storable_> object = Extract(src, share_);
			// add type
         NOTICE2("Expected type", type_);
			Handle_<T_> retval = handle_cast<T_>(object);
			REQUIRE(retval, "Object has wrong type");
			return retval;
		}
	};

	class Reader_ : noncopyable
	{
	public:
		virtual ~Reader_();
		virtual Storable_* Build() const = 0; // handwritten; builds from own data
		virtual Storable_* Build(const Archive::View_& View_, Archive::Built_& share) const = 0;	// builds from archive, NOT from own data
	};

	void Register(const String_& type, const Reader_* extractor);	// we keep the pointer

	// free helper functions
	namespace Utils
	{
		template<class T_> void Set
			(Archive::Store_& dst,
			 const String_& name,
			 const T_& value)
		{
			dst.Child(name) = value;
		}
      // partial specialization for handle to any type
      template<class T_> void Set
         (Archive::Store_& dst,
         const String_& name,
         const Handle_<T_>& value)
      {
         REQUIRE(value, "Can't serialize a null object");   // SetOptional would not get here
         SetStorable(dst, name, dynamic_cast<const Storable_&>(*value));
      }

      // helpers on top of raw Set()
		template<class T_> void SetOptional
			(Archive::Store_& dst,
			 const String_& name,
			 const T_& value)
		{
			if (value != T_())
				Set(dst, name, value);
		}
		// partial specialization for boost::optional
		template<class T_> void SetOptional
			(Archive::Store_& dst,
			const String_& name,
			const boost::optional<T_>& value)
		{
			if (value)
				Set(dst, name, value.get());
		}
		template<class T_> void SetMultiple
			(Archive::Store_& dst,
			 const String_& name,
			 const Vector_<T_>& values)
		{
			for (int ii = 0; ii < values.size(); ++ii)
				Set(dst, name + String::FromInt(ii), values[ii]);
		}

		template<class E_, class T_> void Get
			(const Archive::View_& src,
			const String_& name,
			E_* value,
			const T_& translator)
		{
			*value = translator(src.Child(name));
		}
		template<class E_, class T_> void GetOptional
			(const Archive::View_& src,
			const String_& name,
			E_* value,
			const T_& translator)
		{
			if (src.HasChild(name))
				Get(src, name, value, translator);
		}
		template<class E_, class T_> void GetMultiple
			(const Archive::View_& src,
			const String_& name,
			Vector_<E_>* values,
			const T_& translator)	// element translator
		{
			for (;;)
			{
				String_ childName = name + String::FromInt(values->size());
				if (!src.HasChild(childName))
					break;
				values->push_back(translator(src.Child(childName)));
			}
		}
	}
}

