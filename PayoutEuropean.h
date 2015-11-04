
// simple payouts without backward induction

#pragma once

#include "Payout.h"
#include "BackwardInduction.h"   // ironically, have to include this just to construct empty vectors

// payout in a single stream
template<class T_ = Payout_> class PayoutSingle_ : public T_
{
public:
   const String_& name_;   // of trade and of stream
   Payout_::weights_t StreamWeights() const { return Payout::IdentityWeight(name_); }
protected:
   template<typename... Args_> PayoutSingle_(const String_& name, Args_&&... args) : T_(std::forward<Args_>(args)...), name_(name) {}
};

// payout with no backward actions
template<class T_ = Payout_> class PayoutForward_ : public T_
{
protected:
   template<typename... Args_> PayoutForward_(Args_&&... args) : T_(std::forward<Args_>(args)...) {}
   Vector_<BackwardInduction::Action_> BackwardSteps() const { return Vector_<BackwardInduction::Action_>(); }
};

// payout with only a single event date, so certainly can't have backward actions
template<class T_ = Payout_> class PayoutEuropean_ : public PayoutForward_<T_>
{
public:
   const DateTime_& eventTime_;
   Vector_<DateTime_> EventTimes() const { return Vector::V1(eventTime_); }
protected:
   template<typename... Args_> PayoutEuropean_(const DateTime_& event_time, Args_&&... args) : PayoutForward_<T_>(std::forward<Args_>(args)...), eventTime_(event_time) {}
};

using PayoutSimple_ = PayoutSingle_<PayoutEuropean_<>>;
