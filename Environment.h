
// Carrier (and warning flag) for environment access within functions
// The need for environment should be flagged, and the specific environment provided, at the highest level possible	(honesty)
// Environment information should be extracted, and supporting functions made purely functional, also at the highest level possible (purity)

#pragma once

#include "Vectors.h"
#include "Algorithms.h"

#define _ENV const Environment_* _env
#define XX_ENV_ADD(u, e) Environment::XDecorated_ __xee##u(_env, e)
#define X_ENV_ADD(u, e) XX_ENV_ADD(u, e)
#define ENV_ADD(e) X_ENV_ADD(__COUNTER__, e)
#define ENV_SEED(e) _ENV = nullptr; ENV_ADD(e)
// support ADD and SEED with stack-constructed entries
#define XX_ENV_INST(u, t) t __ei##u; ENV_ADD(__ei##u)
#define X_ENV_INST(u, t) XX_ENV_INST(u, t)
#define ENV_ADD_TYPE(t) X_ENV_INST(__COUNTER__, t)
#define ENV_SEED_TYPE(t) _ENV = nullptr; ENV_ADD_TYPE(t)
// and with factory-constructed entries (macro argument is the factory function)
#define XX_ENV_FACT(u, f) scoped_ptr<Environment::Entry_> __ep##u(f); ENV_ADD(*__ep##u)
#define X_ENV_FACT(u, f) XX_ENV_FACT(u, f)
#define ENV_ADD_PTR(f) X_ENV_FACT(__COUNTER__, f)
#define ENV_SEED_PTR(f) _ENV = nullptr; ENV_ADD_PTR(f)


namespace Environment
{
	struct Entry_ : noncopyable
	{
		virtual ~Entry_();
	};
}

class Environment_ : noncopyable
{
public:
	virtual ~Environment_();

	typedef Environment::Entry_ Entry_;
	struct IterImp_ : noncopyable
	{
		virtual ~IterImp_();
		virtual bool Valid() const = 0;
		virtual IterImp_* Next() const = 0;
		virtual const Entry_& operator*() const = 0;
	};
	struct Iterator_
	{
		Handle_<IterImp_> imp_;
		Iterator_(IterImp_* orphan) : imp_(orphan) {}
		bool IsValid() const { return imp_.get() != 0; }
		void operator++();
		const Entry_& operator*() const { return **imp_; }
	};

	virtual IterImp_* XBegin() const = 0;
	Iterator_ Begin() const { return Iterator_(XBegin()); }
};

namespace Environment
{
	// most generic iterator through entries
	template<typename F_> void Iterate
		(const Environment_* env, F_& func)
	{
		if (env)
			for (auto pe = env->Begin(); pe.IsValid(); ++pe)
				func(*pe);
	}

	// collect entries of a specific type (we do not own the returned pointers)
	template<typename T_> Vector_<const T_*> Collect
		(const Environment_* env)
	{
		Vector_<const T_*> retval;
		Iterate(env, [&](const Handle_<Entry_>& h)
				{if (dyn_ptr(t, const T_*, h.get()))
		         retval.push_back(t); });
		return retval;
	}

	// short-circuit iteration, stops when it finds a value convertible to true
	template<typename F_> auto Find
		(const Environment_* env, F_& func) -> decltype(func(*env->Begin()))
	{
		if (env)
		{
			for (auto pe = env->Begin(); pe.IsValid(); ++pe)
			if (auto ret = func(*pe))
				return ret;
		}
		return 0;
	}
	// specialize to find first entry of a given type
	template<typename T_> const T_* Find
		(const Environment_* env)
	{
		auto func = [](const Entry_& e){ return dynamic_cast<const T_*>(&e); };
		return Find<decltype(func)>(env, func);
	}

	// specific environment, with vector of entries
	class Base_ : public Environment_
	{
		Vector_<Handle_<Entry_>> vals_;
		struct MyIter_ : IterImp_
		{
			const Vector_<Handle_<Entry_>>& all_;
			Vector_<Handle_<Entry_>>::const_iterator me_;
			MyIter_(const Vector_<Handle_<Entry_>>& all, const Vector_<Handle_<Entry_>>::const_iterator& me) : all_(all), me_(me) {}	// wants constructor sugar
			bool Valid() const override
			{
				return me_ != all_.end();
			}
			IterImp_* Next() const override
			{
				return new MyIter_(all_, ::Next(me_));
			}
			const Entry_& operator*() const override
			{
				return **me_;
			}
		};
	public:
		Base_(const Vector_<Handle_<Entry_>>& vals = Vector_<Handle_<Entry_>>()) : vals_(vals) {}	// wants constructor sugar
		MyIter_* XBegin() const override
		{
			return new MyIter_(vals_, vals_.begin());
		}
	};

	// decorate an environment to add one more entry
	// this class needs the parent environment and _ENV pointer to stay in scope
	// use it ONLY through the macros ENV_ADD and ENV_SEED
	class XDecorated_ : public Environment_
	{
		const Environment_*& theEnv_;	// of our scope 
		const Environment_* parent_;
		const Entry_& val_;

		struct I1_ : IterImp_
		{
			const Environment_* parent_;
			const Entry_& val_;
			I1_(const Environment_* parent, const Entry_& val) : parent_(parent), val_(val) {}	// wants constructor sugar
			bool Valid() const { return true; }
			IterImp_* Next() const
			{
				return parent_ ? parent_->XBegin() : 0;
			}
			const Entry_& operator*() const { return val_; }
		};
	public:
		XDecorated_(const Environment_*& the_env, const Entry_& val)
			:
		theEnv_(the_env),
		val_(val)
		{
			parent_ = theEnv_;
			theEnv_ = this;
		}
		~XDecorated_() { theEnv_ = parent_; }

		IterImp_* XBegin() const
		{
			return new I1_(parent_, val_);
		}
	};
}
