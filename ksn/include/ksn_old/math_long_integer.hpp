#pragma once

#ifndef _KSN_MATH_LONG_INTEGER_HPP_
#define _KSN_MATH_LONG_INTEGER_HPP_



#include <ksn/ksn.hpp>
#include <ksn/math_trivial_types.hpp>

#include <stdlib.h>



_KSN_BEGIN

/**
Long integer arithmetic class
Work principals and probably useful notes:
	
	All integers are signed

	Not thread-safe 

	Uses allocator functions instead of template allocators because yes
	(malloc and free by default)
		If you wanna use template allocators, check out wrappers in <ksn/memory.hpp>

	Capacity is measured in 8 byte blocks

	Behaves just like a trivial signed integer type, except when uninitialized or moved by std::move (becomes uninitialized), i.e.:
		ksn::linteger x;
		ksn::linteger y(std::move(x)); //x becomes uninitialized

	Implementation-defined limits:
		sizeof(size_t), capacity limit in units (in power of 2 and human readable forms), capacity limit in hex digits (both forms):
			4,	2^26 - 1, 67 108 863,		2^30 - 16, 1 073 741 808
			8,	2^58 - 1, 2.88 * 10^17,		2^62 - 16, 4.6 * 10^18

		Minimum and maximum values for an object with a capacity of N:
			Min:	2^(8N - 1) - 1
			Max: -2^(8N - 1)

Check out static variables (marked with prefix "gs_"), they may be useful for you
As they are some kind of global "settings" that control work of some code sections
*/
class linteger
{
	struct capacity_construct_t
	{
		size_t value;
	};

public:

	//Performs testing for the library to work properly
	static bool __self_test();



	//Internal stuff
	//May be useful, but you shouldn't use it if you aren't sure exactly what does it do
	//If you aren't, please don't touch these

	//Performs first initialization, takes initial capacity as argument
	void _first_init(size_t);

	//Reaalocates storage and copies old data if passed argument > capacity
	//but leaves new higher bytes uninitialized
	void _extend_no_sign(size_t);

	//Allocates the whole new storage if needed but without copying data
	void _extend_reallocate(size_t);

	//Behaves like linteger::shrink, except that it does nothing if current
	//capacity is bigger by one than the one passed as an argument
	void _shrink(size_t);



	//Private data members
	//Made public to your comfort
	//But don't you dare change them without knowing what you are actually doing
	//P.s. 
	// Keep in mind that it is not supposed at all that you will modify it yourself
	// so if you want do do something but reading values, do it with accuracy

	uint64_t* m_p_data;
	size_t m_capacity;

	//This one is garantued to be valid only after call to linteger::length()
	mutable size_t m_length = 0;



	//Puclic data members
	//Memory management settings
	//Feel free to change it
	
	union
	{
		struct
		{
			bool autoextend_self;
			bool autoextend_result;
			bool autoshrink;
		 } g_flags;
		//Set to zero to make capacity implicitly immutable
		int g_capacity_flags = g_s_initial_capacity_flags;
	};



	//Public global variables
	//Settings
	//Feel free to change it

	//Initial capacity that will be applied to all constructed lintegers
	//Select 0 to not allocate memory by default
	//Default: 4
	static size_t g_s_initial_capacity;

	//Initial capacity flags
	//Bit 0 is for linteger::g_autoextend_self
	//	For example, whether operator++() will extend number to prevent overflow)
	//Bit 16 is for linteger::g_autoextend_result
	//	For example, whether operator+(a, b) will extend the result)
	//Bit 32 is for linteger::g_autoshrink
	//	For example, when you add 256 and -1 result is 255, that may be written into a storage of 1 byte
	//	(it just an exaplme and size of stogare unit is actualy 8 bytes, but hope you got it)
	//Default: 0x10101
	static int g_s_initial_capacity_flags;
	
	//Setting that specifies whether lintegers constructed with a default constructor will be set so zero
	//Default: true
	static bool g_s_init_with_zero;



	//A function called as an allocator
	static void* (*g_s_allocator_function)(size_t);
	//A fucntion called as a deallocator
	static void (*g_s_deallocator_function)(void*, size_t);


	static void restore_default_allocator();
	static void restore_default_deallocator();
	


	//Do ksn::linteger object(ksn::linteger::from_capacity(X));
	//to construct object with initial capacity of X instead of ksn::linteger::g_s_initial_capacity
	static struct capacity_construct_t from_capacity(size_t initial_capacity);
	


	~linteger();

	//Default constructor keeps value uninitialized
	linteger();

	//Construct wth capacity
	linteger(capacity_construct_t);

	linteger(const linteger&);
	linteger(linteger&&) noexcept;

	//Handles signed and unsigend integers and floating point types
	linteger(const trivial_arithmetic_type&, size_t initial_capacity = linteger::g_s_initial_capacity);

	//Does nothing at all, the object keeps uninitialized
	linteger(uninitialized_t);



	linteger& operator=(const linteger& other);
	linteger& operator=(linteger&& other) noexcept;



	void construct_from_signed(int64_t);
	void construct_from_unsigned(uint64_t);
	void construct_from_integer(const trivial_integer_type&);
	void construct_from_double(long double);
	void construct_from_trivial(const trivial_arithmetic_type&);

	

	friend linteger operator+(const linteger& a, const linteger& b);
	friend linteger operator-(const linteger& a, const linteger& b);
	friend linteger operator*(const linteger& a, const linteger& b);
	friend linteger operator/(const linteger& a, const linteger& b);
	friend linteger operator%(const linteger& a, const linteger& b);
	
	friend linteger operator<<(const linteger& a, const linteger& b);
	friend linteger operator>>(const linteger& a, const linteger& b);

	friend linteger operator|(const linteger& a, const linteger& b);
	friend linteger operator&(const linteger& a, const linteger& b);
	friend linteger operator^(const linteger& a, const linteger& b);

	friend bool operator||(const linteger& a, const linteger& b);
	friend bool operator&&(const linteger& a, const linteger& b);

	

	friend linteger operator+(const linteger& a, const trivial_integer_type& b);
	friend linteger operator-(const linteger& a, const trivial_integer_type& b);
	friend linteger operator*(const linteger& a, const trivial_integer_type& b);
	friend linteger operator/(const linteger& a, const trivial_integer_type& b);
	friend linteger operator%(const linteger& a, const trivial_integer_type& b);
	
	friend linteger operator<<(const linteger& a, int64_t b);
	friend linteger operator>>(const linteger& a, int64_t b);

	friend linteger operator|(const linteger& a, const trivial_integer_type& b);
	friend linteger operator&(const linteger& a, const trivial_integer_type& b);
	friend linteger operator^(const linteger& a, const trivial_integer_type& b);

	friend bool operator||(const linteger& a, const trivial_arithmetic_type& b);
	friend bool operator&&(const linteger& a, const trivial_arithmetic_type& b);

	

	friend linteger operator+(const trivial_integer_type& a,  const linteger& b);
	friend linteger operator-(const trivial_integer_type& a,  const linteger& b);
	friend linteger operator*(const trivial_integer_type& a,  const linteger& b);
	friend linteger operator/(const trivial_integer_type& a,  const linteger& b);
	friend linteger operator%(const trivial_integer_type& a,  const linteger& b);
	
	friend linteger operator<<(const trivial_integer_type& a, const linteger& b);
	friend linteger operator>>(const trivial_integer_type& a, const linteger& b);

	friend linteger operator|(const trivial_integer_type& a,  const linteger& b);
	friend linteger operator&(const trivial_integer_type& a,  const linteger& b);
	friend linteger operator^(const trivial_integer_type& a,  const linteger& b);

	friend bool operator||(const trivial_arithmetic_type& a,  const linteger& b);
	friend bool operator&&(const trivial_arithmetic_type& a,  const linteger& b);



	linteger& operator+=(const linteger& other);
	linteger& operator-=(const linteger& other);
	linteger& operator*=(const linteger& other);
	linteger& operator/=(const linteger& other);
	linteger& operator%=(const linteger& other);

	linteger& operator<<=(const linteger& other);
	linteger& operator>>=(const linteger& other);

	linteger& operator|=(const linteger& other);
	linteger& operator&=(const linteger& other);
	linteger& operator^=(const linteger& other);



	linteger& operator+=(const trivial_integer_type& other);
	linteger& operator-=(const trivial_integer_type& other);
	linteger& operator*=(const trivial_integer_type& other);
	linteger& operator/=(const trivial_integer_type& other);
	linteger& operator%=(const trivial_integer_type& other);

	linteger& operator<<=(int64_t other);
	linteger& operator>>=(int64_t other);

	linteger& operator|=(const trivial_integer_type& other);
	linteger& operator&=(const trivial_integer_type& other);
	linteger& operator^=(const trivial_integer_type& other);



	linteger operator++(int);
	linteger operator--(int);
	linteger& operator++();
	linteger& operator--();


	
	//Extends current storage and performs a sign-extend of the current number
	void extend(size_t minimal_capacity);

	//Shrinks current storage to a specified amount of memory
	//P.s. If you pass zero capacity would be set to 1
	void shrink(size_t maximal_capacity);



	//deallocates the number
	void reset();

	//Sets number to 0 in an efficient way
	void zero();

	//Changes sign
	void sign_switch();

	//Performs 1's complement integer negation
	void sign_switch_1s_compliment();



	//Shiht arithmetical right
	void sar(int64_t bits);
	void sar(const linteger& other);

	//Shiht logical right
	void shr(int64_t bits);
	void shr(const linteger& other);

	//Shiht arithmetical left
	void sal(int64_t bits);
	void sal(const linteger& other);

	//Shiht logical left
	void shl(int64_t bits);
	void shl(const linteger& other);



	//Actual number's length
	//(minimal capacity required to store current number)
	//O(1) avg, O(capacity - length) worst
	//(it may or may not be precalculated)
	size_t length() const;

	//Sign bit
	bool sign() const;
	uint32_t sign32() const;
	uint64_t sign64() const;



	//I didn't find any more reasonable way to do convertions to trivial types

	operator int8_t() const;
	operator int16_t() const;
	operator int32_t() const;
	operator int64_t() const;

	operator uint8_t() const;
	operator uint16_t() const;
	operator uint32_t() const;
	operator uint64_t() const;

	operator float() const;
	operator double() const;
	operator long double() const;

	operator char() const;
	operator wchar_t() const;
	operator char16_t() const;
	operator char32_t() const;

	operator bool() const;
};





//Square root
linteger sqrt(const linteger& x);

//Cube root
linteger cbrt(const linteger& x);

//nth degree root
linteger root(const linteger& x, uint32_t degree);


//Raises base to the power of exponent
linteger pow(const linteger& base, uint32_t exponent);


//Base 2 logarithm
size_t log2(const linteger& x);

//Base 10 logarithm
size_t log10(const linteger& x);

//Base 2^n logarithm
size_t log2n(const linteger& x, size_t exponent);

//Base n logarithm
size_t logn(const linteger& x, size_t base);


//Base 2 fp logarithm
long double log2f(const linteger& x);

//Base 10 fp logarithm
long double log10f(const linteger& x);

//Base e fp logarithm
long double ln(const linteger& x);

//Base n fp logarithm
long double lognf(const linteger& x, long double base);



_KSN_END



#endif //_KSN_MATH_LONG_INTEGER_HPP_
