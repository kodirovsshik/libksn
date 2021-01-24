#ifndef  _KSN_MATH_CONSTEXPR_HPP_


#include <ksn/ksn.hpp>
#include <ksn/math_common.hpp>

#include <limits>
#include <stdexcept>





_KSN_BEGIN



template<arithmetic T>
constexpr T exp(T x)
{
	if ((x == x) == false) return x;

	long double result = 1;
	long double num = x;
	long double denom = 1;
	size_t n = 1;

	while (true)
	{
		long double dy = num / denom;
		if ((result + dy) == result) return result;

		result += dy;

		num *= x;
		denom *= ++n;
	}
}




template<std::floating_point T>
constexpr T ln(T x)
{
	if (x >= 2) return -ln(1 / x);
	if (x < -1e-14) return std::numeric_limits<long double>::quiet_NaN();
	if (x <= 1e-14) return -std::numeric_limits<long double>::infinity();

	long double x1 = 1 - x;
	long double num = x1;
	uint64_t denom = 1;

	long double result = 0;

	while (true)
	{
		long double dy = num / denom;
		if (result + dy == result) return -result;

		result += dy;

		num *= x1;
		denom++;
	}
}



template<arithmetic T1, arithmetic T2>
constexpr auto pow(T1 x, T2 y)
{
	return exp<std::common_type_t<T1, T2>>(y * ln(x));
}

template<std::integral T1, std::unsigned_integral T2>
constexpr T1 pow(T1 x, T2 y)
{
	T1 result = 1;
	T2 walker = 1 <<
		(sizeof(T2) == 1) ? log2_8(y) : (
		(sizeof(T2) == 2) ? log2_16(y) : (
		(sizeof(T2) == 4) ? log2_32(y) : (
		(sizeof(T2) == 8) ? log2_64(y)
		)));

	while (walker != 0)
	{
		result *= result;
		if (y & walker)
			result *= x;
		walker >>= 1;
	}

	return result;
}

template<std::integral T1, std::signed_integral T2>
constexpr T1 pow(T1 x, T2 y)
{
	if _KSN_CONSTEXPR_CONDITION(std::is_constant_evaluated())
	{
		if _KSN_CONSTEXPR_CONDITION (y < 0)
		{
			if (x == 0) { return 3 / false; }
			return (x == 1) ? 1 : 0;
		}
	}
	else
	{
		if (y < 0)
		{
			if (x == 0) { return 3 / false; }
			return (x == 1) ? 1 : 0;
		}
	}
	return pow<T1, std::make_unsigned_t<T2>>(x, y);
}



template<std::floating_point T>
constexpr T sqrt(T x)
{
	return pow(x, 0.5l);
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



template<std::floating_point T>
constexpr T root(T x, size_t n)
{
	return pow(x, 1.0L / n);
}

template<std::integral T>
constexpr T root(T x, uint32_t n)
{
	if (n == 0) return std::numeric_limits<T>::max();
	if _KSN_CONSTEXPR(std::is_unsigned_v<T>)
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
	if _KSN_CONSTEXPR(std::is_unsigned_v<T>)
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



constexpr static uint64_t fibonacci(size_t x)
{
	if (x > 92) return INT64_MAX;
	return fibonacci_array[x];
}



template<size_t n>
constexpr static uint64_t fibonacci_v = fibonacci_v<n - 1> +fibonacci_v<n - 2>;

template<>
constexpr static uint64_t fibonacci_v<0> = 0;

template<>
constexpr static uint64_t fibonacci_v<1> = 1;

constexpr static uint64_t fibonacci_array[92] =
{
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



_KSN_END


#endif // ! _KSN_MATH_CONSTEXPR_HPP_
