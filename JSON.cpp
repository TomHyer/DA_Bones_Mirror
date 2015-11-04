
#include "Platform.h"
#include "JSON.h"

#include <fstream>
#include <sstream>
#include "\utils\rapidjson\include\rapidjson\document.h"
#include "\utils\rapidjson\include\rapidjson\writer.h"
#include "\utils\rapidjson\include\rapidjson\prettywriter.h"
#include "\utils\rapidjson\include\rapidjson\filereadstream.h"
#include "\utils\rapidjson\include\rapidjson\filewritestream.h"
#include "Strict.h"

#include "Numerics.h"
#include "DateUtils.h"
#include "DateTimeUtils.h"
#include "Archive.h"

namespace
{
	std::ostream& operator<<(std::ostream& dst, const String_& s) { return dst << reinterpret_cast<const std::string&>(s); }
	static const char* TYPE_LABEL = "~type";
	static const char* TAG_LABEL = "$tag";
	static const char* COLS_LABEL = "cols";
	static const char* VALS_LABEL = "vals";
	typedef rapidjson::GenericValue<rapidjson::UTF8<>> element_t;
	typedef rapidjson::GenericDocument<rapidjson::UTF8<>>::AllocatorType allocator_t;
	using rapidjson::Value;
	inline rapidjson::GenericStringRef<char> LendToJSON(const String_& s)
	{
		return rapidjson::GenericStringRef<char>(s.c_str(), static_cast<int>(s.size()));
	}
// WRITE interface
	// given up on rapidjson, think it is simple enough to just stream out
	// POSTPONED -- add a template parameter determining the checking strategy for funny characters inside strings
		// this simple implementation just assumes there are none (fastest)
		// could have one that wraps, one that errors

	struct XDocStore_ : Archive::Store_
	{
		std::ostream& dst_;
		std::map<const Storable_*, String_>& sharedTags_;
		std::map<String_, std::shared_ptr<XDocStore_>> children_;
		String_ ownName_;
		bool empty_;

		XDocStore_(std::ostream& dst, std::map<const Storable_*, String_>& tags, XDocStore_* parent, const String_& own_name) : dst_(dst), sharedTags_(tags), ownName_(own_name), empty_(true) {}

		// looks like tag has to be the toplevel attribute of an object node
		const char* Prep()
		{
			return empty_ ? "{\n" : ",\n";
		}
		void StoreRefTag(const String_& tag)
		{
			assert(empty_);	// this should always be the first thing written
			dst_ << Prep() <<  "\"" << TAG_LABEL << "\": \"" << tag << "\"";
			empty_ = false;
		}

		bool StoreRef(const Storable_* object) override
		{
			auto ot = sharedTags_.find(object);
			if (ot != sharedTags_.end())
			{
				StoreRefTag(ot->second);
				return true;
			}
			auto tag = ToString(1 + static_cast<int>(sharedTags_.size()));
			sharedTags_.insert(make_pair(object, tag));
			StoreRefTag(tag);
			return false;
		}
		void SetType(const String_& type) override 
		{
			// assert(!empty_);	// should have a tag -- unless it is the toplevel!
			dst_ << Prep() << "\"" << TYPE_LABEL << "\": \"" << type << "\"";
			empty_ = false;
		}
		void Done() override { dst_ << '}'; }
		Store_& Child(const String_& name) override
		{
			std::shared_ptr<XDocStore_>& retval = children_[name];
			if (!retval)
			{
				// assume we are going to write the child immediately
				dst_ << Prep() << "\"" << name << "\": ";
				retval.reset(new XDocStore_(dst_, sharedTags_, this, name));
			}
			return *retval;
		}

		// store atoms
		void operator=(double val) override
		{
			int i = AsInt(val);
			dst_ << (i == val ? String::FromInt(i) : String::FromDouble(val));	// no prep
		}
		void operator=(const String_& val) override
		{
			dst_ << "\"" << val << "\"";
		}
		void operator=(const Date_& val) override 
		{
			operator=(Date::ToString(val));
		}
		void operator=(const Cell_& c)
		{
			switch (c.type_)
			{
			case Cell_::Type_::BOOLEAN:
				operator=(c.b_);
				break;
			case Cell_::Type_::DATE:
				operator=(c.dt_.Date());
				break;
			case Cell_::Type_::DATETIME:
				operator=(c.dt_);
				break;
			case Cell_::Type_::EMPTY:
				operator=(String_());
				break;
			case Cell_::Type_::NUMBER:
				operator=(c.d_);
				break;
			case Cell_::Type_::STRING:
				operator=(c.s_);
				break;
			default:
				THROW("Internal error -- unhandled cell type");
			}
		}

		template<class E_> void SetArray(const Vector_<E_>& val)
		{
			dst_ << "[";
			bool first = true;
			for (const auto& v : val)
			{
				if (!first)
					dst_ << ",";
				first = false;
				operator=(v);
			}
			dst_ << "]";
		}
		void operator=(const Vector_<>& val) override	
		{
			SetArray(val); 
		}
		void operator=(const Vector_<int>& val) override
		{
			SetArray(val);
		}
		void operator=(const Vector_<bool>& val) override
		{
			SetArray(val);
		}
		void operator=(const Vector_<String_>& val) override
		{
			SetArray(val);
		}
		void operator=(const Vector_<Date_>& val) override 
		{
			SetArray(val);
		}
		void operator=(const Vector_<DateTime_>& val) override 
		{
			SetArray(val);
		}

		template<class E_> void SetMatrix(const Matrix_<E_>& val)
		{
			dst_ << "{ \"rows\": " << val.Rows() << ",\n\"vals\": [";
			bool first = true;
			for (int ir = 0; ir < val.Rows(); ++ir)
				for (const auto& v : val.Row(ir))
				{
					if (!first)
						dst_ << ", ";
					first = false;
					operator=(v);
				}
			dst_ << "]}";
		}
		void operator=(const Matrix_<>& val) override { SetMatrix(val); }
		void operator=(const Matrix_<String_>& val) override { SetMatrix(val); }
		void operator=(const Matrix_<Cell_>& val) override { SetMatrix(val); }

		void operator=(const Dictionary_& val) override { operator=(Dictionary::ToString(val)); }
	};

// READ interface
	template<class E_> auto AsVector(element_t& doc, const E_& extract)
		->typename vector_of<decltype(extract(doc))>::type
	{
		REQUIRE(doc.IsArray(), "Can't get a vector value");
		const int n = doc.Size();
		vector_of<decltype(extract(doc))>::type retval(n);
		for (int ii = 0; ii < n; ++ii)
			retval[ii] = extract(doc[ii]);
		return retval;
	}
	// create matrix from vector of values
	template<class E_> Matrix_<E_> AsMatrix(int cols, const Vector_<E_>& vals)
	{
		REQUIRE(cols > 0 && !(vals.size() % cols), "Invalid number of matrix columns");
		const int rows = vals.size() / cols;
		Matrix_<E_> retval(rows, cols);
		for (int ir = 0; ir < rows; ++ir)
			std::copy(&vals[ir * cols], &vals[(ir + 1) * cols], retval.Row(ir).begin());
		return retval;
	}

	// need checked access at every level -- support with "E" element-extractors
	double EDouble(const element_t& doc)
	{
		REQUIRE(doc.IsDouble(), "Can't get a numeric value");
		return doc.GetDouble();
	}
	int EInt(const element_t& doc)
	{
		REQUIRE(doc.IsInt(), "Can't get an integer value");
		return doc.GetInt();
	}
	bool EBool(const element_t& doc)
	{
		REQUIRE(doc.IsBool(), "Can't get a boolean value");
		return doc.GetBool();
	}
	String_ EString(const element_t& doc)
	{
		REQUIRE(doc.IsString(), "Can't get a string value");
		return String_(doc.GetString());
	}		
	Date_ EDate(const element_t& doc)
	{	// worrying about efficiency, so storing dates as Excel-compatible integers
		if (doc.IsInt())
			return Date::FromExcel(doc.GetInt());
		if (doc.IsString())
			return Date::FromString(doc.GetString());
		THROW("Can't get a date value");
	}
	DateTime_ EDateTime(const element_t& doc)
	{	// worrying about efficiency, so storing dates as Excel-compatible doubles
		if (doc.IsDouble())
		{
			double d = doc.GetDouble();
			int i = AsInt(d);
			return DateTime_(Date::FromExcel(i), d - i);
		}
		if (doc.IsString())
			return DateTime::FromString(doc.GetString());
		THROW("Can't get a datetime value");
	}
	Cell_ ECell(const element_t& doc)
	{
		// POSTPONED -- store date/time attribute -- right now they are reconstituted as numbers
		if (doc.IsDouble())
			return Cell_(doc.GetDouble());
		if (doc.IsBool())
			return Cell_(doc.GetBool());
		if (doc.IsString())
			return Cell_(String_(doc.GetString()));
		if (doc.IsNull())
			return Cell_();
		THROW("Invalid cell type");
	}


	// JSON reader based on RapidJSON DOM interface
	struct XDocView_ : Archive::View_
	{
		element_t& doc_;	// may be shared
		mutable std::map<String_, Handle_<XDocView_>> children_;
		XDocView_(element_t& doc) : doc_(doc) {}

		// shared-object part of Archive::View_ interface
		// we will do this by having a Tag() which is empty except for shared objects (same as Splat)
		String_ AfterPrefix(char prefix) const
		{
			if (doc_.IsString() && doc_.GetStringLength() > 1 && doc_.GetString()[0] == prefix)
			{
				return String_(doc_.GetString()).substr(1);
			}
			return String_();
		}
		String_ Tag() const
		{
			if (doc_.HasMember(TAG_LABEL))
				return EString(doc_[TAG_LABEL]);
			return String_();
		}
		Handle_<Storable_>& Known(Archive::Built_& built) const override // returns a reference within 'built'
		{
			return built.known_[Tag()];
		}

		// direct storage of atoms
		double AsDouble() const override { return EDouble(doc_); }
		int AsInt() const override { return EInt(doc_); }
		bool AsBool() const override { return EBool(doc_); }
		Date_ AsDate() const override { return EDate(doc_); }
		String_ AsString() const override { return EString(doc_); }
		Dictionary_ AsDictionary() const override
		{
			return Dictionary::FromString(AsString());
		}

		Vector_<> AsDoubleVector() const override { return AsVector(doc_, EDouble); }
		Vector_<int> AsIntVector() const override { return AsVector(doc_, EInt); }
		Vector_<bool> AsBoolVector() const override { return AsVector(doc_, EBool); }
		Vector_<String_> AsStringVector() const override { return AsVector(doc_, EString); }
		Vector_<Date_> AsDateVector() const override { return AsVector(doc_, EDate); }
		Vector_<DateTime_> AsDateTimeVector() const override { return AsVector(doc_, EDateTime); }

		// matrix storage is:
		//		the matrix has a child "cols" that gives the number of columns
		//		and a child "vals" that gives all the values, scanning each row in turn
		Matrix_<> AsDoubleMatrix() const override
		{
			return AsMatrix(EInt(doc_[COLS_LABEL]), AsVector(doc_[VALS_LABEL], EDouble));
		}
		Matrix_<String_> AsStringMatrix() const override
		{
			return AsMatrix(EInt(doc_[COLS_LABEL]), AsVector(doc_[VALS_LABEL], EString));
		}
		Matrix_<Cell_> AsCellMatrix() const override
		{
			return AsMatrix(EInt(doc_[COLS_LABEL]), AsVector(doc_[VALS_LABEL], ECell));
		}

		// object query interface
		String_ Type() const override
		{
			if (doc_.HasMember(TYPE_LABEL))
				return EString(doc_[TYPE_LABEL]);
			return String_();
		}
		element_t& XChild(const String_& name) const { return doc_[name.c_str()]; }
		bool HasChild(const String_& name) const override { return doc_.HasMember(name.c_str()); }
		const View_& Child(const String_& name) const override
		{
			Handle_<XDocView_>& retval = children_[name];
			if (retval.Empty())
				retval.reset(new XDocView_(XChild(name)));
			return *retval;
		}

		void Unexpected(const String_&) const override {}	// ignore extra children
	};
}	// leave local

Handle_<Storable_> JSON::ReadString(const String_& src, bool quiet)
{
	NOTE("Extracting object from JSON string");
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseDefaultFlags>(src.c_str());
	XDocView_ task(doc);
	Archive::Built_ built;
	return Archive::Extract(task, built);
}

Handle_<Storable_> JSON::ReadFile(const String_& filename, bool quiet)
{
	FILE* fp = fopen(filename.c_str(), "rb");	// POSTPONED -- use "r" on non-Windows platforms
	REQUIRE(fp, "File not found:'" + filename + "'");
	char buffer[8192];
	rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
	rapidjson::Document doc;
	doc.ParseStream<rapidjson::kParseDefaultFlags>(is);
	fclose(fp);
	XDocView_ task(doc);
	Archive::Built_ built;
	return Archive::Extract(task, built);
}

void JSON::WriteFile(const Storable_& object, const String_& filename)
{
	std::ofstream dst(filename.c_str());
	std::map<const Storable_*, String_> tags;
	XDocStore_ task(dst, tags, nullptr, String_());
	object.Write(task);
}

String_ JSON::WriteString(const Storable_& object)
{
	std::stringstream retval;
	std::map<const Storable_*, String_> tags;
	XDocStore_ task(retval, tags, nullptr, String_());
	object.Write(task);
	return retval.str();
}

