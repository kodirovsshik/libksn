#include <ksn/ksn.hpp>

#include <limits>
#include <stdexcept>

_KSN_BEGIN

constexpr static long double ceexp(long double x)
{
	if (x != x) return x;

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

constexpr static long double celn(long double x)
{
	if (x >= 2) return -celn(1 / x);
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

constexpr static long double cepow(long double x, long double y)
{
	return ceexp(y * celn(x));
}

constexpr static long double ceroot(long double x)
{
	return cepow(x, 0.5);
}

constexpr static uint64_t fib(size_t x)
{
	constexpr long double root5 = (double)ceroot(5);
	constexpr long double phi = (1 + root5) / 2, phiJ = (1 - root5) / -2;
#ifdef _KSN_COMPILER_MSVC
	#pragma warning(push)
	#pragma warning(disable : 26451)
#endif
	const long double negator = 2 * int(x % 2) - 1;
#ifdef _KSN_COMPILER_MSVC
	#pragma warning(pop)
#endif
	return uint64_t((cepow(phi, x)) / root5 + 0.5);
}

//constexpr static uint64_t fibonacci(uint8_t n)
//{
//	if (n > 92) throw std::overflow_error("92 is max");
//
//	uint64_t a = 0, b = 1;
//	if (n & 1)
//	{
//		n -= 1;
//		a = 1;
//	}
//	while (n)
//	{
//		b += a;
//		a += b;
//		n -= 2;
//	}
//	return a;
//}


template<size_t n>
constexpr uint64_t fibonacci_v = fibonacci_v<n - 1> +fibonacci_v<n - 2>;

template<>
constexpr uint64_t fibonacci_v<0> = 0;

template<>
constexpr uint64_t fibonacci_v<1> = 1;

static constexpr uint64_t fibonacci[92] =
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
