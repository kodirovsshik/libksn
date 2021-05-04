
#ifndef _KSN_MATH_PPLF_HPP_
#define _KSN_MATH_PPLF_HPP_


#include <ksn/ksn.hpp>

#include <concepts>
#include <numeric>

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

	 void shift_left(size_t bits)
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
	 void shift_right(size_t bits)
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

	 static void adjust_exponents(pplf* __restrict pin1, pplf* __restrict pin2)
	{
		//size_t less_leading_bits;
		//size_t bits1 = pin1->leading_zeros(), bits2 = pin2->leading_zeros();

		//if (bits1 < bits2)
		//	less_leading_bits = bits1;
		//else
		//	less_leading_bits = bits2;

		//pin1->shift_left(less_leading_bits);
		//pin2->shift_left(less_leading_bits);

		 pin1->shift_left(pin1->leading_zeros());
		 pin2->shift_left(pin2->leading_zeros());

		if (pin1->exponent < pin2->exponent)
		{
			pin1->shift_right(pin2->exponent - pin1->exponent);
		}
		else if (pin1->exponent > pin2->exponent)
		{
			pin2->shift_right(pin1->exponent - pin2->exponent);
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
				pout->shift_right(1);
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

		if  (numeric_limits<T>::digits >= 64)
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
		if constexpr (_KSN_IS_DEBUG_BUILD)
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
		this->shift_right(this->trailing_zeros());

		constexpr pplf c1 = []{ ksn::pplf x{}; x.digits[0] = x.digits[1] = 0xB4B4B4B4B4B4B4B4; x.exponent = -126; x.sign = false; return x; }();
		constexpr pplf c2 = []{ ksn::pplf x{}; x.digits[0] = x.digits[1] = 0xF0F0F0F0F0F0F0F0; x.exponent = -127; x.sign = true; return x; }();

		int shift = 128 - this->leading_zeros() + this->exponent;
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
};

static_assert(sizeof(pplf) == 24);


#define KSN_PPLF_DIG 38
#define KSN_PPLF_DECIMAL_DIG 39



_KSN_END


#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif //!_KSN_MATH_PPLF_HPP_
