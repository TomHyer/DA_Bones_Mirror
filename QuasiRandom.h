
// quasi-random sequences base class and resource protection
// use of quasi-random sequences is a global resource which must not be duplicated

#pragma once

namespace QuasiRandom
{
	class SequenceSet_ : noncopyable
	{
	public:
		virtual ~SequenceSet_();
		virtual int Size() const = 0;
		virtual void Next(Vector_<>* dst) = 0;
		virtual SequenceSet_* TakeAway(int subsize) = 0;
	};
}
