
#ifndef  _KSN_MATH_CONSTEXPR_HPP_
#define _KSN_MATH_CONSTEXPR_HPP_


#include <ksn/ksn.hpp>
#include <ksn/math_constants.hpp>

#include <limits>
#include <stdexcept>
#include <numeric>

#undef min
#undef max





_KSN_BEGIN



_KSN_DETAIL_BEGIN

#define ksn_log_repeat1(x) (x)
#define ksn_log_repeat2(x) ksn_log_repeat1(x), ksn_log_repeat1(x)
#define ksn_log_repeat4(x) ksn_log_repeat2(x), ksn_log_repeat2(x)
#define ksn_log_repeat8(x) ksn_log_repeat4(x), ksn_log_repeat4(x)
#define ksn_log_repeat16(x) ksn_log_repeat8(x), ksn_log_repeat8(x)
#define ksn_log_repeat32(x) ksn_log_repeat16(x), ksn_log_repeat16(x)
#define ksn_log_repeat64(x) ksn_log_repeat32(x), ksn_log_repeat32(x)
#define ksn_log_repeat128(x) ksn_log_repeat64(x), ksn_log_repeat64(x)

constexpr int8_t __log2_lookup_table8[256] =
{
	INT8_MIN, //some sort of -infinity for log(0)
	ksn_log_repeat1(0), ksn_log_repeat2(1), ksn_log_repeat4(2), ksn_log_repeat8(3),
	ksn_log_repeat16(4), ksn_log_repeat32(5), ksn_log_repeat64(6), ksn_log_repeat128(7)
};

#undef ksn_log_repeat1
#undef ksn_log_repeat2
#undef ksn_log_repeat4
#undef ksn_log_repeat8
#undef ksn_log_repeat16
#undef ksn_log_repeat32
#undef ksn_log_repeat64
#undef ksn_log_repeat128

_KSN_DETAIL_END



template<std::unsigned_integral T>
constexpr int ilog2(T x)
{
	//I could have generalized this but i already had implementations for all integer sizes and i was too lazy to do something about it

	if constexpr (sizeof(T) == 1)
	{
		return detail::__log2_lookup_table8[x];
	}
	else if constexpr (sizeof(T) == 2)
	{
		uint8_t t;
		return (t = (x >> 8)) ? (8 + detail::__log2_lookup_table8[t]) : (detail::__log2_lookup_table8[x]);
	}
	else if constexpr (sizeof(T) == 4)
	{
		uint16_t t1, t2;

		if ((t2 = (x >> 16)))
		{
			return (t1 = (t2 >> 8)) ? (16 + 8 + detail::__log2_lookup_table8[t1]) : (16 + detail::__log2_lookup_table8[t2]);
		}
		else
		{
			return (t2 = (x >> 8)) ? (8 + detail::__log2_lookup_table8[t2]) : (detail::__log2_lookup_table8[x]);
		}
	}
	else if constexpr (sizeof(T) == 8)
	{
		uint64_t t1, t2;

		if ((t1 = (x >> 32)))
		{
			if ((t2 = (t1 >> 16)))
			{
				return (t1 = (t2 >> 8)) ? (32 + 16 + 8 + detail::__log2_lookup_table8[t1]) : (32 + 16 + detail::__log2_lookup_table8[t2]);
			}
			else
			{
				return (t2 = (t1 >> 8)) ? (32 + 8 + detail::__log2_lookup_table8[t2]) : (32 + detail::__log2_lookup_table8[t1]);
			}
		}
		else
		{
			if ((t2 = (x >> 16)))
			{
				return (t1 = (t2 >> 8)) ? (16 + 8 + detail::__log2_lookup_table8[t1]) : (16 + detail::__log2_lookup_table8[t2]);
			}
			else
			{
				return (t1 = (x >> 8)) ? (8 + detail::__log2_lookup_table8[t1]) : detail::__log2_lookup_table8[x];
			}
		}
	}
	else
	{
		_ksn_ct_unreachable("In my days we had no 128 bit integers");
	}
}

template<std::signed_integral T>
constexpr int ilog2(T x)
{
	if (x <= 0) return INT_MIN;

	return ilog2((std::make_unsigned_t<T>)x);
}



template<std::signed_integral T>
constexpr T isqrt(T x)
{
	if (x <= 0) return INT_MIN;
	return isqrt((std::make_unsigned_t<T>)x);
}
template<std::unsigned_integral T>
constexpr T isqrt(T n)
{
	if (n == T(-1))
		return (1 << (sizeof(T) / 2)) - 1;

	T x = n, y = 1;
	while (x > y)
	{
		x = (x + y) / 2;
		y = n / x;
	}
	return x;
}



template<class T>
constexpr T abs(T x) noexcept
{
	if (x < 0) return -x;
	return x;
}
template<std::signed_integral T>
constexpr T abs(T x) noexcept
{
	if (T < 0) return -x;
	return x;
}
template<std::unsigned_integral T>
constexpr T abs(T x) noexcept
{
	return x;
}

template<class T>
constexpr T uabs(T x) noexcept
{
	return ksn::abs(x);
}
template<std::signed_integral T>
constexpr T uabs(T x) noexcept
{
	return ksn::abs(x);
}
template<std::unsigned_integral T>
constexpr T uabs(T x) noexcept
{
	using sT = std::make_signed_t<T>;
	return (T)ksn::uabs((sT)x);
}





template<class T, bool use_dx>
constexpr T _sin(const T& x, const T* p_dx) noexcept
{
	if (x != x)
		return x;

	//TODO: fix
	//x = ksn::fmod(x, T(2 * KSN_PId));
	if (x > T(KSN_PI)) return _sin<T, use_dx>(x - T(2 * KSN_PI), p_dx);

	T result = x;
	T sqr = -x * x;
	T current = x;

	size_t i = 1;
	while (1)
	{
		i += 2;
		current *= sqr / (i * (i - 1));
		T new_result = result + current;
		if constexpr (use_dx)
		{
			if (ksn::abs(new_result - result) <= *p_dx)
				return new_result;
		}
		else
		{
			if (new_result == result)
				return new_result;
		}
		result = new_result;
	}
}

template<class T>
constexpr T sin(const T& x) noexcept
{
	return _sin<T, false>(x, nullptr);
}
template<class T>
constexpr T sin(const T& x, const T& dx) noexcept
{
	return _sin<T, true>(x, &dx);
}

template<class T>
constexpr T cos(const T& x) noexcept
{
	return ksn::sin(x - T(KSN_PI / 2));
}
template<class T>
constexpr T cos(const T& x, const T& dx) noexcept
{
	return ksn::sin(x - T(KSN_PI / 2), dx);
}





template<class T, bool use_dx>
constexpr T _exp(const T& x, const T* p_dx)
{
	if (x != x)
		return x;

	long double result = 1;
	long double num = x;
	long double denom = 1;
	size_t n = 1;

	while (true)
	{
		long double new_result = result + num / denom;
		if constexpr (use_dx)
		{
			if (ksn::abs(new_result - result) < *p_dx)
				return new_result;
		}
		else
		{
			if (new_result == result)
				return new_result;
		}

		result = new_result;

		num *= x;
		denom *= ++n;
	}
}

template<class T>
constexpr T exp(const T& x)
{
	return _exp<T, false>(x, nullptr);
}
template<class T>
constexpr T exp(const T& x, const T& dx)
{
	return _exp<T, true>(x, &dx);
}





template<class T, bool use_dx>
constexpr T _ln(const T& x, const T* p_dx)
{
	if (x != x)
		return x;

	if (x >= 1.5) return -_ln<T, use_dx>(1 / x, p_dx);
	if (x < 0) return std::numeric_limits<T>::quiet_NaN();
	if (x == 0) return -std::numeric_limits<T>::infinity();
	if (x <= 1e-2) return T(-5) + _ln<T, use_dx>(x * exp<T>(5), p_dx);

	long double x1 = 1 - x;
	long double num = x1;
	uint64_t denom = 1;

	long double result = 0;

	while (true)
	{
		long double new_result = result + num / denom;
		if constexpr (use_dx)
		{
			if (ksn::abs(new_result - result) < *p_dx)
				return -new_result;
		}
		else
		{
			if (new_result == result)
				return -new_result;
		}

		result = new_result;

		num *= x1;
		denom++;
	}
}
template<class T>
constexpr T ln(const T& x)
{
	return _ln<T, false>(x, nullptr);
}
template<class T>
constexpr T ln(const T& x, const T& dx)
{
	return _ln<T, true>(x, &dx);
}




template<class T1, class T2>
constexpr auto pow(T1 x, T2 y)
{
	return ksn::exp<std::common_type_t<T1, T2>>(y * ksn::ln(x));
}
template<class T1, class T2, class T3>
constexpr auto pow(T1 x, T2 y, T3 d)
{
	return ksn::exp<std::common_type_t<T1, T2>>(y * ksn::ln(x, d), d);
}

template<std::integral T1, std::unsigned_integral T2>
constexpr T1 pow(T1 x, T2 y)
{
	T1 result = 1;
	if (!y) return result;
	
	T2 walker = 1 << ilog2(y);

	while (walker != 0)
	{
		result *= result;
		if (y & walker)
			result *= x;
		walker >>= 1;
	}

	return result;
}

template<class T1, std::integral T2>
constexpr T1 pow(T1 x, T2 y)
{
	using T2u = std::make_signed_t<T2>;

	if constexpr (std::signed_integral<T2>)
	{
		if (y < 0)
		{
			if (y == std::numeric_limits<T2>::min())
				return (x == T1(0)) ? NAN : 0;

			return pow<T1, T2u>(T1(1) / x, T2u(-y));
		}
	}

	return pow(x, (T2u)y);
}

template<std::integral T1, std::signed_integral T2>
constexpr T1 pow(T1 x, T2 y)
{
	if (y < 0)
	{
		if (x == 0) { x = 0; x /= x; return x; }
		return (x == 1) ? 1 : 0;
	}
	return ksn::pow<T1, std::make_unsigned_t<T2>>(x, y);
}





template<class T>
constexpr T sqrt(T x)
{
	return ksn::pow(x, 0.5l);
}
template<class T>
constexpr T sqrt(T x, T dx)
{
	return ksn::pow(x, 0.5l, dx);
}

template<class T>
constexpr T cbrt(T x)
{
	return ksn::pow(x, 1.l / 3);
}
template<class T>
constexpr T cbrt(T x, T dx)
{
	return ksn::pow(x, 1.l / 3, dx);
}

template<std::integral T>
constexpr T sqrt(T x)
{
	if (x < 2) return x;
	constexpr static inline T maxroot = (T(1 << (sizeof(T) * 4)) - 1);
	if (x >= maxroot * maxroot) return maxroot;

	T l = 0, r = x;

	while (true)
	{
		T m = (l + r) / 2;

		if (m > (T(1) << (sizeof(T) * 4)) - 1)
		{
			r = m;
			continue;
		}

		T m2 = m * m;
		if (m2 == x) return m;
		if (m2 > x)
		{
			r = m;
		}
		else //m*m<x
		{
			if ((m + 1) * (m + 1) > x) return m;
			l = m;
		}
	}
}





template<class T>
constexpr T root(T x, size_t n)
{
	return pow(x, 1.0L / n);
}
template<class T>
constexpr T root(T x, size_t n, T dx)
{
	return pow(x, 1.0L / n, dx);
}

template<std::integral T>
constexpr T root(T x, uint32_t n)
{
	if (n == 0) return std::numeric_limits<T>::max();
	if constexpr(std::is_unsigned_v<T>)
	{
		if (x < 2) return x;
	}
	else
	{
		if (n & 1)
		{
			if (x > -2 && x < 2) return x;
		}
		else
		{
			if (x < 2) return x;
		}
	}

	//max T value that will not overflow when raised to n power
	T maxroot = (T(1) << (sizeof(T) * 8 / n)) - 1;
	if (x >= pow(maxroot, n)) return maxroot;

	T l, r;
	if constexpr(std::is_unsigned_v<T>)
	{
		l = 0; r = x;
	}
	else
	{
		if (x > 0)
		{
			l = 0; r = x;
		}
		else
		{
			if ((x & 1) == 0) _KSN_RAISE(std::domain_error(""));
			l = x; r = 0;
		}
	}

	while (true)
	{
		T m = (l + r) / 2;
		if (m > maxroot)
		{
			r = m;
			continue;
		}
		T mn = pow(m, n);
		if (mn == x) return m;
		if (mn > x)
		{
			r = m;
			continue;
		}
		if (pow(m + 1, n) > x) return m;
		l = m;
	}
}



template<class T>
constexpr T fabs(T x)
{
	if (x < 0) return -x;
	return x;
}

template<class T, std::integral iT>
constexpr T exp2(iT n)
{
	T x = 1;
	while (n > 0)
	{
		x *= 2;
		n--;
	}
	while (n < 0)
	{
		x /= 2;
		n++;
	}
	return x;
}

template<class T>
constexpr T trunc(T x)
{
	if (x < 0) return -trunc(-x);
	if (x < 1) return 0;
	
	int revert_power = 0;
	do
	{
		++revert_power;
		x /= 2;
	} while (x >= 1);

	constexpr int mantissa_digits = std::numeric_limits<T>::digits;

	if (revert_power >= mantissa_digits)
		return x;

	x *= exp2<T>(mantissa_digits);
	x = T((uint64_t)x & ~((uint64_t(1) << (mantissa_digits - revert_power)) - 1));
	x *= exp2<T>(revert_power - mantissa_digits);

	return x;
}
template<class T>
constexpr T floor(T x)
{
	if (x < 0)
		return -trunc(-x) - 1;
	return trunc(x);
}

template<class T>
constexpr T fmod(T x, T mod)
{
	T q = trunc(x / mod);
	return x - q * mod;
}
template<class T>
constexpr T fmod1(T x, T mod)
{
	T q = floor(x / mod);
	return x - q * mod;
}

template<class T>
constexpr T fdiv(T x, T div)
{
	return trunc(x / div);
}
template<class T>
constexpr T fdiv1(T x, T div)
{
	return floor(x / div);
}

template<class T>
std::pair<T, T> fdivmod(T x, T div)
{
	T q = trunc(x / div);
	return { q, x - q * div };
}
template<class T>
std::pair<T, T> fdivmod1(T x, T div)
{
	T q = floor(x / div);
	return { q, x - q * div };
}




template<size_t n>
constexpr static uint64_t fibonacci_v = fibonacci_v<n - 1> + fibonacci_v<n - 2>;

template<>
constexpr static uint64_t fibonacci_v<0> = 0;

template<>
constexpr static uint64_t fibonacci_v<1> = 1;

constexpr static uint64_t fibonacci_array[92] =
{ //I actually wrore a program that printed that array for me
	fibonacci_v<00>, fibonacci_v<01>, fibonacci_v<02>,
	fibonacci_v<03>, fibonacci_v<04>, fibonacci_v<05>,
	fibonacci_v<06>, fibonacci_v<07>, fibonacci_v< 8>,
	fibonacci_v< 9>, fibonacci_v<10>, fibonacci_v<11>,
	fibonacci_v<12>, fibonacci_v<13>, fibonacci_v<14>,
	fibonacci_v<15>, fibonacci_v<16>, fibonacci_v<17>,
	fibonacci_v<18>, fibonacci_v<19>, fibonacci_v<20>,
	fibonacci_v<21>, fibonacci_v<22>, fibonacci_v<23>,
	fibonacci_v<24>, fibonacci_v<25>, fibonacci_v<26>,
	fibonacci_v<27>, fibonacci_v<28>, fibonacci_v<29>,
	fibonacci_v<30>, fibonacci_v<31>, fibonacci_v<32>,
	fibonacci_v<33>, fibonacci_v<34>, fibonacci_v<35>,
	fibonacci_v<36>, fibonacci_v<37>, fibonacci_v<38>,
	fibonacci_v<39>, fibonacci_v<40>, fibonacci_v<41>,
	fibonacci_v<42>, fibonacci_v<43>, fibonacci_v<44>,
	fibonacci_v<45>, fibonacci_v<46>, fibonacci_v<47>,
	fibonacci_v<48>, fibonacci_v<49>, fibonacci_v<50>,
	fibonacci_v<51>, fibonacci_v<52>, fibonacci_v<53>,
	fibonacci_v<54>, fibonacci_v<55>, fibonacci_v<56>,
	fibonacci_v<57>, fibonacci_v<58>, fibonacci_v<59>,
	fibonacci_v<60>, fibonacci_v<61>, fibonacci_v<62>,
	fibonacci_v<63>, fibonacci_v<64>, fibonacci_v<65>,
	fibonacci_v<66>, fibonacci_v<67>, fibonacci_v<68>,
	fibonacci_v<69>, fibonacci_v<70>, fibonacci_v<71>,
	fibonacci_v<72>, fibonacci_v<73>, fibonacci_v<74>,
	fibonacci_v<75>, fibonacci_v<76>, fibonacci_v<77>,
	fibonacci_v<78>, fibonacci_v<79>, fibonacci_v<80>,
	fibonacci_v<81>, fibonacci_v<82>, fibonacci_v<83>,
	fibonacci_v<84>, fibonacci_v<85>, fibonacci_v<86>,
	fibonacci_v<87>, fibonacci_v<88>, fibonacci_v<89>,
	fibonacci_v<90>, fibonacci_v<91>
};


constexpr static uint64_t fibonacci(size_t x)
{
	if (x >= 92) return 0;
	return fibonacci_array[x];
}





_KSN_DETAIL_BEGIN

template<class T, bool use_dx>
constexpr T _atan(const T& x, const T* dx)
{
	if (x != x)
		return x;

	if (x <= -1)
	{
		return _atan<T, use_dx>(-x, dx);
	}

	T sqr, sum, current;
	if (abs(x) < 1)
	{
		sqr = -1 / (x * x);
		sum = current = x;;
	}
	else
	{
		sqr = -x * x;
		sum = T(KSN_PId / 2);
		current = -1 / x;
	}

	size_t i = 1;
	while (1)
	{
		i += 2;
		current *= sqr;
		T new_sum = sum + current / i;
		if constexpr (use_dx)
		{
			if (new_sum == sum)
				return new_sum;
		}
		else
		{
			if (ksn::abs(new_sum - sum) <= dx)
				return new_sum;
		}
		sum = new_sum;
	}
}
template<class T, bool use_dx>
constexpr T _atan2(const T& y, const T& x, const T* dx) noexcept
{
	static constexpr T pi = T(KSN_PI);
	bool small_x;

	if constexpr (use_dx)
		small_x = ksn::abs(x) < *dx;
	else
		small_x = x == 0;

	if (small_x)
	{
		if (y > 0) return pi / 2;
		if (y < 0) return -pi / 2;
		//y == 0
		return 0;
	}

	T atan = _atan<T, use_dx>(y / x, dx);

	if (x > 0) return atan;

	//x < 0
	if (y >= 0) return atan + pi;
	//y < 0
	return atan - pi;
}

_KSN_DETAIL_END

template<class T>
constexpr T atan(const T& x) noexcept
{
	return detail::_atan<T, false>(x, nullptr);
}
template<class T>
constexpr T atan(const T& x, const T* dx) noexcept
{
	return detail::_atan<T, true>(x, dx);
}

template<class T>
constexpr T atan2(const T& y, const T& x) noexcept
{
	return detail::_atan2<T, false>(y, x, nullptr);
}
template<class T>
constexpr T atan2(const T& y, const T& x, const T& dx) noexcept
{
	return detail::_atan2<T, true>(y, x, &dx);
}



_KSN_END


#endif // ! _KSN_MATH_CONSTEXPR_HPP_
