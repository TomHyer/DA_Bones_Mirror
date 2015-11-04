
#include "Platform.h"
#include "ReceivePay.h"
#include "Strict.h"

#include "Vectors.h"
#include "Exceptions.h"
#include "MG_RecPay_enum.inc"

int RecPay_::RecSign() const
{
	return val_ == Value_::REC ? 1 : -1;
}

RecPay_ RecPay_::operator-() const
{
	return val_ == Value_::REC ? Value_::PAY : Value_::REC;
}

