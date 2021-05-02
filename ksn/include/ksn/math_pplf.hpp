
#ifndef _KSN_MATH_PPLF_HPP_
#define _KSN_MATH_PPLF_HPP_


#include <ksn/ksn.hpp>

#include <concepts>
#include <numeric>

#include <cmath>

#include <intrin.h>


_KSN_BEGIN



//Poor programmer's long float
struct pplf
{
private:
	static void _mul_digits_big(const pplf* pin1, const pplf* pin2, pplf* pout)
	{
		uint64_t h01, h10;
		uint64_t l11;
		uint64_t h11; //temporary for result high digits
		uint64_t temp; //temporary for result low digits

		_umul128(pin1->digits[0], pin2->digits[1], &h01);
		_umul128(pin1->digits[1], pin2->digits[0], &h10);
		l11 = _umul128(pin1->digits[1], pin2->digits[1], &h11);

		unsigned char carry;
		carry = _addcarry_u64(0, h01, h10, &temp);
		h11 += carry;

		carry = _addcarry_u64(0, temp, l11, &temp);
		if (carry)
		{
			if (h11 == UINT64_MAX) _KSN_UNLIKELY
			{
				temp >>= 1;
				temp |= (h11 << 63);
				h11 = 1ui64 << 63;
			}
			else
			{
				h11++;
			}
		}
		pout->digits[0] = temp;
		pout->digits[1] = h11;

		pout->exponent = pin1->exponent + pin2->exponent + 128;
		pout->sign = pin1->sign ^ pin2->sign;
	}
	static void _mul_digits_medium(const pplf* pin1, const pplf* pin2, pplf* pout)
	{
		uint64_t l10, h10, h00;

		_umul128(pin1->digits[0], pin2->digits[0], &h00);
		l10 = _umul128(pin1->digits[1], pin2->digits[0], &h10);

		h10 += _addcarry_u64(0, l10, h00, &pout->digits[0]);
		pout->digits[1] = h10;

		pout->exponent = pin1->exponent + pin2->exponent + 64;
		pout->sign = pin1->sign ^ pin2->sign;
	}
	static void _mul_digits(const pplf* in1, const pplf* in2, pplf* out)
	{
		if (in1->digits[1] != 0 && in2->digits[1] != 0) return _mul_digits_big(in1, in2, out);
		if (in1->digits[1] != 0 && in2->digits[1] == 0) return _mul_digits_medium(in1, in2, out);
		if (in2->digits[1] != 0 && in1->digits[1] == 0) return _mul_digits_medium(in2, in1, out);

		out->digits[0] = _umul128(in1->digits[0], in2->digits[0], &out->digits[1]);
		out->exponent = in1->exponent + in2->exponent;
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
		size_t less_leading_bits;
		size_t bits1 = pin1->leading_zeros(), bits2 = pin2->leading_zeros();

		if (bits1 < bits2)
			less_leading_bits = bits1;
		else
			less_leading_bits = bits2;

		pin1->shift_left(less_leading_bits);
		pin2->shift_left(less_leading_bits);

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
	constexpr pplf(T x)
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
	operator T() const noexcept
	{
		return std::exp2((T)this->exponent) * (this->digits[0] + std::exp2(T(64)) * this->digits[1]) * (this->sign ? -1 : 1);
	}

	template<std::integral T>
	operator T() const noexcept
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
		pplf::_mul_digits(&a, &b, &result);
		return result;
	}
	pplf& operator*=(pplf other) noexcept
	{
		pplf this_copy(*this);
		pplf::_mul_digits(&this_copy, &other, this);
		return *this;
	}

	friend pplf operator/(pplf a, pplf b) noexcept
	{
		pplf result;
		b.invert();
		pplf::_mul_digits(&a, &b, &result);
		return result;
	}
	pplf& operator/=(pplf other) noexcept
	{
		pplf this_copy(*this);
		other.invert();
		pplf::_mul_digits(&this_copy, &other, this);
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

	void invert() noexcept
	{
		this->shift_right(this->trailing_zeros());

		pplf result;
		result.digits[0] = 1;
		result.exponent = -this->exponent - 1;
		result.sign = this->sign;

		for (size_t i = 0; i < 32; ++i)
		{
			pplf new_result = result + result * (pplf(1) - *this * result);
			if (new_result == result) break;
			result = new_result;
		}

		*this = result;
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



_KSN_END



#endif //!_KSN_MATH_PPLF_HPP_
