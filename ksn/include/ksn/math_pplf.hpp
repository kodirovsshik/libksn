
#ifndef _KSN_MATH_PPLF_HPP_
#define _KSN_MATH_PPLF_HPP_


#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>

#include <concepts>
#include <numeric>
#include <string>

#include <cmath>

#include <intrin.h>


#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 26451)
#endif


_KSN_BEGIN



//Poor programmer's long float
struct pplf
{
private:
	static void multiply(const pplf* in1, const pplf* in2, pplf* out)
	{
		if (in2->exponent == 0x3FFFFFFFFFFFFFFF) //inf or nan
			return multiply(in2, in1, out);

		if (in1->exponent == 0x3FFFFFFFFFFFFFFF)
		{
			if (in2->is_zero())
			{
				*out = *in1;
				out->exponent = 0x3FFFFFFFFFFFFFFF;
				return;
			}

		}


		uint64_t product[4];
		unsigned char carry;
		uint64_t l00, l01, l10, l11, h00, h01, h10, h11;

		l00 = _umul128(in1->digits[0], in2->digits[0], &h00);
		l01 = _umul128(in1->digits[0], in2->digits[1], &h01);
		l10 = _umul128(in1->digits[1], in2->digits[0], &h10);
		l11 = _umul128(in1->digits[1], in2->digits[1], &h11);

		carry = 0;
		carry = _addcarry_u64(carry, l00, 0, &product[0]);
		carry = _addcarry_u64(carry, l01, l10, &product[1]);
		carry = _addcarry_u64(carry, h01, h10, &product[2]);
		carry = _addcarry_u64(carry, 0, h11, &product[3]);

		carry = 0;
		carry = _addcarry_u64(carry, product[1], h00, &product[1]);
		carry = _addcarry_u64(carry, product[2], l11, &product[2]);
		carry = _addcarry_u64(carry, product[3], 0, &product[3]);


		unsigned long shift_over = 0, shift;
		if constexpr (false) {}
		else if (product[3] == 0 && product[2] == 0 && product[1] == 0 && product[0] == 0)
		{
			out->digits[0] = out->digits[1] = 0;
			out->exponent = 0;
			out->sign = in1->sign ^ in2->sign;
			return;
		}
		else if (product[3] == 0 && product[2] == 0 && product[1] == 0)
		{
			product[3] = product[0];
			product[2] = 0;
			product[1] = 0;
			product[0] = 0;
			shift_over = 192;
		}
		else if (product[3] == 0 && product[2] == 0)
		{
			product[3] = product[1];
			product[2] = product[0];
			product[1] = 0;
			product[0] = 0;
			shift_over = 128;
		}
		else if (product[3] == 0)
		{
			product[3] = product[2];
			product[2] = product[1];
			product[1] = product[0];
			product[0] = 0;
			shift_over = 64;
		}

		if (_BitScanReverse64(&shift, product[3]))
			shift = 63 - shift;
		else
			shift = 64;

		if (shift)
		{
			product[3] = __shiftleft128(product[2], product[3], (unsigned char)shift);
			product[2] = __shiftleft128(product[1], product[2], (unsigned char)shift);
			product[1] = __shiftleft128(product[0], product[1], (unsigned char)shift);
			//product[0] <<= shift; //we don't care about these digits as they are lost anyways 
		}

		carry = 0;
		carry = _addcarry_u64(carry, product[1], 0x8000000000000000, &product[1]);
		carry = _addcarry_u64(carry, product[2], 0, &product[2]);
		carry = _addcarry_u64(carry, product[3], 0, &product[3]);

		if (carry)
		{
			product[2] = __shiftright128(product[2], product[3], 1);
			product[3] >>= 1;
			product[3] |= 0x8000000000000000;
			shift -= 1;
		}
		
		shift += shift_over;

		out->digits[0] = product[2];
		out->digits[1] = product[3];
		out->exponent = in1->exponent + in2->exponent - shift + 128;
		out->sign = in1->sign ^ in2->sign;
	}

public:
	void _shift_left(size_t bits)
	{
		if (bits >= 128) _KSN_UNLIKELY
		{
			this->digits[1] = this->digits[0] = 0;
		}
		else if (bits >= 64) _KSN_UNLIKELY
		{
			this->digits[1] = this->digits[0] << (bits - 64);
			this->digits[0] = 0;
		}
		else if (bits) _KSN_LIKELY
		{
#ifdef _KSN_COMPILER_MSVC
			this->digits[1] = __shiftleft128(this->digits[0], this->digits[1], (uint8_t)bits);
#else
			this->digits[1] <<= bits;
			this->digits[1] |= this->digits[0] >> (64 - bits);
#endif
			this->digits[0] <<= bits;
		}
		else return;
		this->exponent -= bits;
	}
	void _shift_right(size_t bits)
	{
		if (bits >= 128) _KSN_UNLIKELY
		{
			this->digits[1] = this->digits[0] = 0;
		}
		else if (bits >= 64) _KSN_UNLIKELY
		{
			this->digits[0] = this->digits[1] >> (bits - 64);
			this->digits[1] = 0;
		}
		else if (bits) _KSN_LIKELY
		{
#ifdef _KSN_COMPILER_MSVC
			this->digits[0] = __shiftright128(this->digits[0], this->digits[1], (uint8_t)bits);
#else
			this->digits[0] >>= bits;
			this->digits[0] |= this->digits[1] << (64 - bits);
#endif
			this->digits[1] >>= bits;
		}
		else return;
		this->exponent += bits;
	}

private:
	static void adjust_exponents(pplf* __restrict pin1, pplf* __restrict pin2)
	{
		//size_t less_leading_bits;
		//size_t bits1 = pin1->leading_zeros(), bits2 = pin2->leading_zeros();

		//if (bits1 < bits2)
		//	less_leading_bits = bits1;
		//else
		//	less_leading_bits = bits2;

		//pin1->_shift_left(less_leading_bits);
		//pin2->_shift_left(less_leading_bits);

		 pin1->_shift_left(pin1->leading_zeros());
		 pin2->_shift_left(pin2->leading_zeros());

		if (pin1->exponent < pin2->exponent)
		{
			pin1->_shift_right(pin2->exponent - pin1->exponent);
		}
		else if (pin1->exponent > pin2->exponent)
		{
			pin2->_shift_right(pin1->exponent - pin2->exponent);
		}
	}

	static void add(pplf* __restrict pin1, pplf* __restrict pin2, pplf* __restrict pout)
	{
		pplf::adjust_exponents(pin1, pin2);

		pout->exponent = pin1->exponent;

		if (pin1->sign == pin2->sign)
		{
			unsigned char carry = 0;
			carry = _addcarry_u64(carry, pin1->digits[0], pin2->digits[0], &pout->digits[0]);
			carry = _addcarry_u64(carry, pin1->digits[1], pin2->digits[1], &pout->digits[1]);

			if (carry)
			{
				pout->_shift_right(1);
				pout->digits[1] |= 0x8000000000000000;
			}
		}
		else
		{
			if ((pin2->digits[1] > pin1->digits[1]) ||
				(pin2->digits[1] == pin1->digits[1] && pin2->digits[0] > pin1->digits[0])
				)
				std::swap(pin1, pin2);

			unsigned char carry = 0;
			carry = _subborrow_u64(carry, pin1->digits[0], pin2->digits[0], &pout->digits[0]);
			carry = _subborrow_u64(carry, pin1->digits[1], pin2->digits[1], &pout->digits[1]);

			//carry is always 0 as we substract smaller value from a bigger one
			if (carry) throw;
		}

		pout->sign = pin1->sign;
	}


	template<std::integral T>
	std::make_signed_t<T> to_int() const noexcept
	{
		using sT = std::make_signed_t<T>;

		if (this->exponent == 0) return (sT)this->digits[0];
		if (this->exponent < 0)
		{
			if (this->exponent <= -128) return (sT)0;
			if (this->exponent <= -64) return (sT)(this->digits[1] >> (-this->exponent - 64));
			//exp = [-63; -1]
			//-exp = [1; 63]
			return (sT)(this->digits[0] >> -this->exponent) | (sT)(this->digits[1] << (64 - -this->exponent));
		}
		if (this->exponent > 64) return (sT)0;
		//exp = [1; 63]
		return (sT)(this->digits[0] << this->exponent);
	}


	/*
	Flags:
	1 << 0: Force sign
	1 << 1: Force + sign for 0
	1 << 2: Force exponent sign
	1 << 3: Force + sign for 0 for exponent
	1 << 4: Use comma instead of point //wtf why would anyone ever use this
	1 << 5: Uppercase exponent letter
	1 << 6: Force floating point/comma even if no fraction part would be written
	1 << 7: Don't remove trailing zeros in 'g' format
	*/
	template<class std_string> requires(is_specialization_v<std_string, std::basic_string>)
	std_string to_basic_string(size_t precision, char format, int flags) const
	{
		throw 0;
		if (format != 'f' && format != 'g' && format != 'e') return "";

		pplf copy = *this;
		copy.sign = 0;

		bool saved_sign = this->sign;

		pplf int_part, frac_part;
		frac_part = modf(copy, &int_part);

		if (format == 'g')
		{
			precision += (precision == 0);

		}

		//TODO: implement

		throw;
		std_string result;

		return result;
	}

public:
	uint64_t digits[2];
	int64_t exponent : 63;
	uint64_t sign : 1;


	constexpr pplf()
	{
		this->digits[0] = this->digits[1] = this->exponent = this->sign = 0;
	}

	template<std::floating_point T>
	/*constexpr*/ pplf(T x)
	{
		if (x == 0)
		{
			memset(this, 0, sizeof(*this));
			return;
		}
		if (x < 0)
		{
			this->sign = 1;
			x = -x;
		}
		else this->sign = 0;
		
		using std::numeric_limits;

		if constexpr (numeric_limits<T>::digits >= 64)
		{
			this->digits[0] = 0;
			this->digits[1] = uint64_t(1) << (numeric_limits<T>::digits - 64);
		}
		else
		{
			this->digits[0] = uint64_t(1) << numeric_limits<T>::digits;
			this->digits[1] = 0;
		}

		{
			T mantiss;
			int exp;

			using std::frexp;
			using std::modf;

			mantiss = frexp(x, &exp);

			T ci, cf;
			cf = modf(this->digits[1] * mantiss, &ci);

			this->digits[1] = _addcarry_u64(0, uint64_t(this->digits[0] * mantiss), uint64_t(18446744073709551616.0 * cf), &this->digits[0]);
			this->digits[1] += (uint64_t)ci;

			this->exponent = exp - numeric_limits<T>::digits;
		}
	}

	template<std::unsigned_integral T>
	constexpr pplf(T x)
	{
		this->digits[0] = x;
		this->digits[1] = 0;
		this->exponent = 0;
		this->sign = 0;
	}

	template<std::signed_integral T>
	constexpr pplf(T x)
	{
		if (x == std::numeric_limits<T>::min())
		{
			//corner case
			this->digits[0] = 0;
			this->digits[1] = 1;
			this->exponent = 0;
			this->sign = 1;
			return;
		}
		if (x < 0)
		{
			this->sign = 1;
			x = -x;
		}
		else this->sign = 0;

		this->digits[0] = x;
		this->digits[1] = 0;
		this->exponent = 0;
	}



	template<std::floating_point T>
	constexpr operator T() const noexcept
	{
		return std::exp2((T)this->exponent) * (this->digits[0] + std::exp2(T(64)) * this->digits[1]) * (this->sign ? -1 : 1);
	}

	template<std::integral T>
	constexpr operator T() const noexcept
	{
		auto x = this->sign ? -this->to_int<T>() : this->to_int<T>();
		return (T)x;
	}

	template<class std_string> requires(is_specialization_v<std_string, std::basic_string>)
	operator std_string() const noexcept
	{

	}



	static constexpr pplf e() noexcept
	{
		pplf e;
		e.digits[0] = 0xafdc5620273d3cf1;
		e.digits[1] = 0xadf85458a2bb4a9a;
		e.exponent = -126;
		e.sign = 0;
		return e;
	}

	static constexpr pplf pi() noexcept
	{
		pplf pi;
		pi.digits[0] = 0xc4c6628b80dc1cd1;
		pi.digits[1] = 0xc90fdaa22168c234;
		pi.exponent = -126;
		pi.sign = 0;
		return pi;
	}

	static constexpr pplf phi() noexcept
	{
		pplf pi;
		pi.digits[0] = 0xf9ce60302e76e41a;
		pi.digits[1] = 0xcf1bbcdcbfa53e0a;
		pi.exponent = -127;
		pi.sign = 0;
		return pi;
	}


	static constexpr pplf inf() noexcept
	{
		pplf x;
		x.digits[0] = x.digits[1] = 0xFFFFFFFFFFFFFFFF;
		x.exponent = 0x3FFFFFFFFFFFFFFF; //62 bits set
		x.sign = 0;
		return x;
	}


	constexpr pplf operator+() const noexcept
	{
		return *this;
	}
	constexpr pplf operator-() const noexcept
	{
		pplf result(*this);
		result.sign ^= 1;
		return result;
	}


	friend pplf operator+(pplf a, pplf b) noexcept
	{
		pplf result;
		pplf::add(&a, &b, &result);
		return result;
	}
	pplf& operator+=(pplf other) noexcept
	{
		pplf this_copy(*this);
		pplf::add(&this_copy, &other, this);
		return *this;
	}

	friend pplf operator-(pplf a, pplf b) noexcept
	{
		pplf result;
		b.sign ^= 1;
		pplf::add(&a, &b, &result);
		return result;
	}
	pplf& operator-=(pplf other) noexcept
	{
		pplf this_copy(*this);
		other.sign ^= 1;
		pplf::add(&this_copy, &other, this);
		return *this;
	}

	friend pplf operator*(pplf a, pplf b) noexcept
	{
		pplf result;
		if constexpr (false && _KSN_IS_DEBUG_BUILD)
		{
			double fa = a, fb = b;
			pplf::multiply(&a, &b, &result);
			double f = result;
			printf("%g * %g yields %g\n", fa, fb, f);
		}
		else
			pplf::multiply(&a, &b, &result);
		return result;
	}
	pplf& operator*=(pplf other) noexcept
	{
		pplf this_copy(*this);
		pplf::multiply(&this_copy, &other, this);
		return *this;
	}

	friend pplf operator/(pplf a, pplf b) noexcept
	{
		pplf result;
		b.invert();
		pplf::multiply(&a, &b, &result);
		return result;
	}
	pplf& operator/=(pplf other) noexcept
	{
		pplf this_copy(*this);
		other.invert();
		pplf::multiply(&this_copy, &other, this);
		return *this;
	}



	constexpr bool is_zero() const noexcept
	{
		return this->digits[0] == 0 && this->digits[1] == 0;
	}


	int leading_zeros() const noexcept
	{
		unsigned long index;
		if (this->digits[1] == 0)
		{
			if (_BitScanReverse64(&index, this->digits[0]))
			{
				return 127 - index;
			}
			else return 128;
		}
		else
		{
			_BitScanReverse64(&index, this->digits[1]);
			return 63 - index;
		}
	}

	int trailing_zeros() const noexcept
	{
		unsigned long result;
		if (this->digits[0] == 0)
		{
			if (_BitScanForward64(&result, this->digits[1]))
				return (int)(64 + result);
			else
				return 128;
		}
		else
		{
			_BitScanForward64(&result, this->digits[0]);
			return (int)result;
		}
	}

	 //Newton–Raphson division
	void invert() noexcept
	{
		this->_shift_right(this->trailing_zeros());

		constexpr pplf c1 = []{ ksn::pplf x{}; x.digits[0] = x.digits[1] = 0xB4B4B4B4B4B4B4B4; x.exponent = -126; x.sign = false; return x; }();
		constexpr pplf c2 = []{ ksn::pplf x{}; x.digits[0] = x.digits[1] = 0xF0F0F0F0F0F0F0F0; x.exponent = -127; x.sign = true; return x; }();

		int64_t shift = 128 - this->leading_zeros() + this->exponent;
		this->exponent -= shift;

		bool reserved_sign = this->sign;
		this->sign = 0;

		pplf current = *this * c2 + c1;

		//Wikipedia promised me 5 would be enough
		for (size_t i = 0; i < 5; ++i)
		{
			current = current + current * (pplf(1) - *this * current);
		}

		*this = current;
		this->exponent -= shift;
		this->sign = reserved_sign;
	}


	bool operator==(pplf other) const noexcept
	{
		if (this->digits[0] == 0 && this->digits[1] == 0 && other.digits[0] == 0 && other.digits[1] == 0) return true;

		if (this->sign != other.sign) return false;

		pplf current(*this);
		pplf::adjust_exponents(&current, &other);

		return current.digits[0] == other.digits[0] && current.digits[1] == other.digits[1] && current.exponent == other.exponent;
	}
	bool operator!=(const pplf& other) const noexcept
	{
		return !(this->operator==(other));
	}
	bool operator<(const pplf& other) const noexcept
	{
		pplf diff = *this - other;
		return diff.sign && (diff.digits[0] || diff.digits[1]);
	}
	bool operator<=(const pplf& other) const noexcept
	{
		pplf diff = *this - other;
		return diff.sign || !(diff.digits[0] || diff.digits[1]);
	}
	bool operator>(const pplf& other) const noexcept
	{
		return !(this->operator<=(other));
	}
	bool operator>=(const pplf& other) const noexcept
	{
		return !(this->operator<(other));
	}


	friend pplf floor(pplf x) noexcept;

	friend pplf fmod(pplf x, pplf y) noexcept;
	//Returns remainder, stores quotient in *q
	friend pplf divmod(pplf x, pplf y, pplf* q) noexcept;

	//Mathematically correct division remainder
	friend pplf fmod1(pplf x, pplf y) noexcept;
	//Mathematically correct division remainder, returns the remainder, stores quotient in *q
	friend pplf divmod1(pplf x, pplf y, pplf* q) noexcept;

	friend pplf modf(pplf x, pplf* int_part) noexcept;
	
	friend pplf modf_int(pplf x) noexcept;
};

static_assert(sizeof(pplf) == 24);



pplf frexp(pplf x, int64_t* exp)
{
	int64_t shift = x.exponent - x.leading_zeros() + 128;
	x.exponent -= shift;
	if (exp) *exp = shift;
	return x;
}

pplf floor(pplf x) noexcept
{
	if (x.exponent < 0) //otherwise x is already integer
		x._shift_right(-x.exponent);
	return x;
}

pplf modf(pplf x, pplf* int_part) noexcept
{
	pplf origin = x;

	x = floor(x);

	if (int_part)
		*int_part = x;

	return origin - x;
}
pplf modf_int(pplf x) noexcept
{
	if (x.exponent < 0) //otherwise x is already integer
		x._shift_right(-x.exponent);
	return x;
}

pplf fmod(pplf x, pplf y) noexcept
{
	return divmod(x, y, nullptr);
}
pplf divmod(pplf x, pplf y, pplf* q) noexcept
{
	pplf truncated;
	truncated = x / y;
	modf(truncated, &truncated);

	if (q) *q = truncated;
	return x - truncated * y;
}

pplf fmod1(pplf x, pplf y) noexcept
{
	if (y.sign)
	{
		y.sign ^= 1;
		x.sign ^= 1;
	}

	x = fmod(x, y);
	if (x.sign)
		x += y;
	return x;
}
pplf divmod1(pplf x, pplf y, pplf* q) noexcept
{
	if (y.sign)
	{
		y.sign ^= 1;
		x.sign ^= 1;
	}

	x = divmod(x, y, q);
	if (x.sign)
	{
		x += y;
		if (q) *q -= 1;
	}
	return x;
}

pplf exp(pplf x) noexcept
{
	/*
	1) if x < 0 return 1/exp(-x)
	2) decompose x info fractional and integer parts F and I respectively
	3) Find e^F using Taylor series expansion and point x=0.5
	4) Find e^I using binary power raising
	5) return e^F * E^I
	*/
	
	constexpr static auto small_exp_f = []
	(pplf x) -> pplf
	{
		constexpr static pplf root_e = []() { pplf x; x.digits[1] = 0xd3094c70f034de4b; x.digits[0] = 0x96ff7d5b6f99fcd9; x.exponent = -127; x.sign = false; return x; }();
		constexpr static pplf half = []() { pplf x; x.digits[1] = 0; x.digits[0] = 1; x.exponent = -1; x.sign = false; return x; }();
		
		x -= half;
		
		pplf sum = 1 + x;
		pplf current = x;

		int upper = (int)ceilf(25 * cbrtf(fabsf(float(x) - 0.5f))) + 8;
		for (int n = 2; n <= upper; ++n)
		{
			current *= x / n;
			sum += current;
		}

		return sum * root_e;
	};

	constexpr static auto small_exp_i = []
	(pplf x) -> pplf
	{
		pplf result = 1;
		if (x == 0) return result;

		x._shift_right(x.trailing_zeros());
		if (x.exponent > 0) return pplf::inf();

		if ((x.digits[1]) || (x.digits[0] & 0x8000000000000000)) return pplf::inf();
		for (int i = 62; i >= 0; --i)
		{
			x *= x;
			if (x.digits[0] & (uint64_t(1) << i))
				result *= x;
		}

		return result;
	};

	bool invert = x.sign;
	x.sign = false;

	pplf I, F;
	F = modf(x, &I);

	F = small_exp_f(F);
	I = small_exp_i(I);

	x = I * F;
	if (invert) x.invert();
	return x;
}

pplf log(pplf x) noexcept
{
	if (x.sign || x.is_zero()) return -pplf::inf();
	/*if (false && x > 1)
	{
		x.invert();
		return -log(x);
	}
	pplf y = x - pplf(1);

	while (1)
	{
		pplf temp = exp(y);
		pplf dy = (x - temp) / (x + temp) * 2;
		dy.exponent += 1;

		if (y + dy == y) return y;
		y += dy;
	}*/
	
	int64_t shift;
	x = frexp(x, &shift);

	constexpr static pplf ln2 = [] { pplf x; x.digits[0] = 0xc9e3b39803f2f6af; x.digits[1] = 0xb17217f7d1cf79ab; x.exponent = -128; x.sign = false; return x; }();
	constexpr static pplf ln3o4 = [] { pplf x; x.digits[0] = 0xdf5bb3b60554e151; x.digits[1] = 0x934b1089a6dc93c1; x.exponent = -129; x.sign = true; return x; }();
	constexpr static pplf c_4o3 = [] {pplf x; x.digits[0] = x.digits[1] = 0xaaaaaaaaaaaaaaaa; x.exponent = -127; x.sign = false; return x; }();

	double t;

	pplf multiplier = 1 - x * c_4o3;
	pplf current = multiplier;
	pplf initial_guess = ln3o4;

	t = multiplier;
	t = initial_guess;
	//current = 1
	//current *= multiplier
	initial_guess -= current;
	t = initial_guess;
	for (int n = 2; n <= 4; ++n)
	{
		current *= multiplier;
		initial_guess -= current / n;
		t = initial_guess;
	}

	pplf result = initial_guess;
	for (int i = 3; i > 0; --i)
	{
		pplf temp = exp(result);
		temp = (x - temp) / (x + temp);
		temp.exponent++;
		result += temp;
		t = result;
	}

	result += shift * ln2;
	t = result;
	return result;
	//return result + shift * ln2;
}
pplf log(pplf x, pplf base) noexcept
{
	return log(x) / log(base);
}

pplf pow(pplf x, pplf y) noexcept
{
	if (x.is_zero()) return (y >= 0) ? pplf(y.is_zero()) : pplf::inf();
	return exp(y * log(x));
}

pplf abs(pplf x) noexcept
{
	x.sign = false;
	return x;
}





#define KSN_PPLF_DIG 38
#define KSN_PPLF_DECIMAL_DIG 39



_KSN_END


#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif //!_KSN_MATH_PPLF_HPP_
