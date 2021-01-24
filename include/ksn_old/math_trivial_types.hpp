#pragma once


#ifndef _KSN_MATH_TRIVIALS_HPP_
#define _KSN_MATH_TRIVIALS_HPP_


#include <ksn/ksn.hpp>




_KSN_BEGIN


struct trivial_signed_type
{
	int64_t m_value;

	 trivial_signed_type(int8_t);
	 trivial_signed_type(int16_t);
	 trivial_signed_type(int32_t);
	 trivial_signed_type(int64_t);
};

struct trivial_unsigned_type
{
	uint64_t m_value;

	 trivial_unsigned_type(uint8_t);
	 trivial_unsigned_type(uint16_t);
	 trivial_unsigned_type(uint32_t);
	 trivial_unsigned_type(uint64_t);
};

struct trivial_floating_type
{
	long double m_value;

	 trivial_floating_type(float);
	 trivial_floating_type(double);
	 trivial_floating_type(long double);
};

struct trivial_integer_type
{
	union
	{
		int64_t m_value_signed;
		uint64_t m_value_unsigned;
	};

	bool m_is_signed;

	 trivial_integer_type(int8_t);
	 trivial_integer_type(int16_t);
	 trivial_integer_type(int32_t);
	 trivial_integer_type(int64_t);

	 trivial_integer_type(uint8_t);
	 trivial_integer_type(uint16_t);
	 trivial_integer_type(uint32_t);
	 trivial_integer_type(uint64_t);
};

struct trivial_arithmetic_type
{
	union
	{
		int64_t m_value_signed;
		uint64_t m_value_unsigned;
		long double m_value_floating;
	};

	//0 = signed
	//1 = unsigned
	//2 = floating
	uint8_t m_type;
	
	 trivial_arithmetic_type(int8_t);
	 trivial_arithmetic_type(int16_t);
	 trivial_arithmetic_type(int32_t);
	 trivial_arithmetic_type(int64_t);

	 trivial_arithmetic_type(uint8_t);
	 trivial_arithmetic_type(uint16_t);
	 trivial_arithmetic_type(uint32_t);
	 trivial_arithmetic_type(uint64_t);

	 trivial_arithmetic_type(float);
	 trivial_arithmetic_type(double);
	 trivial_arithmetic_type(long double);

	 trivial_arithmetic_type(trivial_integer_type);
};


_KSN_END





#endif // !_KSN_MATH_TRIVIALS_HPP_
