
#pragma once

#ifndef _KSN_MATH_COMMON_HPP_
#define _KSN_MATH_COMMON_HPP_


#include <ksn/ksn.hpp>
#include <ksn/math_complex.hpp>

#include <vector>
#include <stdexcept>
#include <concepts>
#include <complex>

#include <cmath>

#include <locale.h>





_KSN_BEGIN



std::vector<double> polynomial_multiplication(const std::vector<double>& v1, const std::vector<double>& v2);

std::vector<double> polynomial_interpolation(const std::vector<std::pair<double, double>>& points);




//template<typename T>
//void DFT(const T* in, ksn::complex* out, size_t n)
//{
//#ifndef _KSN_NO_DFT_INT64_WARNING
//	static_assert(sizeof(long double) == 16 || !(std::is_same_v<T, uint64_t> | std::is_same_v<T, int64_t>),
//		"Warning: starting from 2^52 not every signle integer value can be represented with double precision FP type. "
//		"Keep this in mind when calling a DFT of a int64 array. "
//		"To suppress this warning, define _KSN_NO_DFT_INT64_WARNING before any <ksn/math*.hpp> inclusion. "
//		"You see this warning because sizeof(long double) on your system is not 16 so you better know what you are doing"
//		);
//#endif
//
//	const long double angle_coefficient = 2 * KSN_PI / (long double)(n);
//
//	for (size_t i = 0; i < n; ++i)
//	{
//		ksn::complex current = 0;
//
//		for (size_t j = 0; j < n; ++j)
//		{
//			long double angle = (long double)(uint64_t(i) * j) * angle_coefficient;
//			current += in[j] * ksn::complex(cos(angle), sin(angle));
//		}
//
//		out[i] = current;
//	}
//}

//template<typename T>
//void IDFT(const ksn::complex* in, T* out, size_t n)
//{
//#ifndef _KSN_NO_DFT_INT64_WARNING
//	static_assert(sizeof(long double) == 16 || !(std::is_same_v<T, uint64_t> | std::is_same_v<T, int64_t>),
//		"Warning: starting from 2^52 not every signle integer value can be represented with double precision FP type. "
//		"Keep this in mind when calling an IDFT of a int64 array. "
//		"To suppress this warning, define _KSN_NO_DFT_INT64_WARNING before any <ksn/math*.hpp> inclusion. "
//		"You see this warning because sizeof(long double) on your system is not 16 so you better know what you are doing"
//		);
//#endif
//	const double angle_coefficient = -2 * KSN_PI / (long double)(n);
//
//	for (size_t i = 0; i < n; ++i)
//	{
//		ksn::complex current = 0;
//
//		for (size_t j = 0; j < n; ++j)
//		{
//			long double angle = angle_coefficient * i * j; //-V113
//			current += in[j] * ksn::complex(cos(angle), sin(angle));
//		}
//
//		out[i] = T((current / (long double)(n)).data[0] + 0.5L);
//	}
//}



//I do need to take the input by copy
std::vector<int64_t> solve_integer_polynomial(std::vector<int64_t> coefficients);




_KSN_DETAIL_BEGIN

template<typename int_type>
constexpr const char* _fp_int_cast_printf_format_helper = nullptr;

template<>
constexpr const char* _fp_int_cast_printf_format_helper<uint8_t> = "%hhu %n";
template<>
constexpr const char* _fp_int_cast_printf_format_helper<int8_t> = "%hhi %n";
template<>
constexpr const char* _fp_int_cast_printf_format_helper<uint16_t> = "%hu %n";
template<>
constexpr const char* _fp_int_cast_printf_format_helper<int16_t> = "%hi %n";
template<>
constexpr const char* _fp_int_cast_printf_format_helper<uint32_t> = "%u %n";
template<>
constexpr const char* _fp_int_cast_printf_format_helper<int32_t> = "%i %n";
template<>
constexpr const char* _fp_int_cast_printf_format_helper<uint64_t> = "%llu %n";
template<>
constexpr const char* _fp_int_cast_printf_format_helper<int64_t> = "%lli %n";

_KSN_DETAIL_END



template<typename int_type>
int_type fp_int_cast(long double x, unsigned precision = LDBL_DIG)
{

	if constexpr(!std::is_integral_v<int_type>)
	{
		return int_type(x);
	}
	else
	{
		if (x > std::numeric_limits<int_type>::max() || x < std::numeric_limits<int_type>::min())
		{
			return int_type(0);
		}

		if (x + 1 == x)
		{
			return int_type(x);
		}

		char buffer[1024];
		snprintf(buffer, 1024, "%.*Lg", precision, x);

		int read;

		int_type result;
		sscanf_s(buffer, detail::_fp_int_cast_printf_format_helper<int_type>, &result, &read);

		if (buffer[read] != '\0')
		{
			return int_type(x);
		}

		return result;
	}
}






template<class fp_t, class c_t> requires(!std::is_integral_v<fp_t>)
constexpr int solve_quadratic(fp_t a, fp_t b, fp_t c, fp_t(*p_roots)[2])
{
	using std::sqrt;

	b *= -0.5 * a;
	c *= a;
	
	fp_t D = b * b - c;
	if (D < 0) return 0;

	(*p_roots)[0] = b + sqrt(D);
	(*p_roots)[1] = b - sqrt(D);
	return 2;
}

template<class T> requires(!std::is_integral_v<T>)
constexpr int solve_quadratic(complex<T> a, complex<T> b, complex<T> c, complex<T>(*p_roots)[2])
{
	a = 1 / a;
	b *= -0.5 * a;
	c *= a;

	complex<T> d = b * b - c;
	(*p_roots)[0] = b + sqrt(d);
	(*p_roots)[1] = b - sqrt(d);

	return 2;
}


template<class fp_t>
constexpr fp_t solve_cubic_principal(fp_t a, fp_t b, fp_t c, fp_t d)
{
	using std::sqrt;
	using std::cbrt;
	using std::atan2;
	using std::cos;

	b /= a;
	c /= a;
	d /= a;

	fp_t p = c - b * b / 3;
	fp_t q = b * (2 * b * b / 27 - c / 3) + d;

	q /= -2; p /= 3;
	fp_t A = q * q + p * p * p;

	if (A > 0)
	{
		A = sqrt(A);
		return cbrt(q + A) + cbrt(q - A) - b / 3;
	}
	else
	{
		A = -A;
		return 2 * pow(q * q + A, fp_t(1) / 6) * cos(atan2(sqrt(A), q) / 3);
	}
}

template<std::floating_point fp_t>
int solve_cubic(fp_t a, fp_t b, fp_t c, fp_t d, fp_t(*p_roots)[3])
{
	using std::sqrt;
	using std::cbrt;

	b /= a;
	c /= a;
	d /= a;

	fp_t p = c - b * b / 3;
	fp_t q = b * (2 * b * b / 27 - c / 3) + d;

	q /= -2; p /= 3;
	fp_t A = q * q + p * p * p;

	if (A > 0)
	{
		A = sqrt(A);
		(*p_roots)[0] = cbrt(q + A) + cbrt(q - A) - b / 3;
		return 1;
	}
	else
	{
		ksn::complex<fp_t> B(A), roots1[3], roots2[3];
		B = sqrt(B);

		ksn::roots(q + B, 3, roots1);
		ksn::roots(q - B, 3, roots2);

		for (int i = 0; i < 3; ++i)
		{
			(*p_roots)[i] = (roots1[i] + roots2[i]).real - b / 3;
		}
		return 3;
	}
}

template<std::floating_point T>
int solve_qartic(T a, T b, T c, T d, T e, T(*p_roots)[4]);



/*
 (m)
S
  n
Stigling number of the first kind
*/
template<typename return_type = long long>
return_type stirling_s1(size_t n, size_t m)
{
	if constexpr(!(std::is_integral_v<return_type> || std::is_floating_point_v<return_type>) || std::is_same_v<return_type, bool>)
	{
		_KSN_RAISE(_STD invalid_argument("Incorrect return type passed to stirling_s1"));
	}
	else
	{
		if (m > n)
		{
			_KSN_RAISE(_STD invalid_argument("stirling_s1: m must be <= n"));
		}

		if (m == 0)
		{
			return 0;
		}

		if (n == m)
		{
			return 1;
		}

		if (m == 1)
		{
			return_type result = 2;
			for (size_t i = 3; i < size_t(n); ++i)
			{
				result *= i;
			}
			return result;
		}

		return_type* arr = new return_type[n + 1];

		memset(arr + 1, 0, sizeof(return_type) * (n));
		arr[0] = 1;

		for (size_t _m = 1; _m <= m; ++_m)
		{
			return_type saved = arr[_m];
			arr[_m] = 1;
			for (size_t _n = _m + 1; _n <= n; ++_n)
			{
				return_type temp = arr[_n];
				arr[_n] = saved - (_n - 1) * arr[_n - 1];
				saved = temp;
			}
		}

		return_type result = arr[n];

		delete[] arr;

		return result;
	}
}



template<typename T>
bool compare_equal_with_precision(T&& a, T&& b, long double precision)
{
	T difference = a - b;
	return difference < precision && difference > -precision;
}



template<std::floating_point T>
T sign(T x)
{
	if (x == 0) return 0;
	return x < 0 ? -1 : 1;
}





long double lambert_W0(long double x);

long double lambert_W_n1(long double x);

//template<std::floating_point T>
//complex<T> lambert_W(complex<T> z, int n = 0);



/*
Flags:
Bit 0: allow '.' to be used as decimal point
Bit 1: allow ',' to be used as decimal point
Bit 2: allow use 'e' as exponent part begin ( mantiss*10^exponent )
Bit 3: allow use 'E' as exponent part begin ( mantiss*10^exponent )
Bit [4:5]:
	0 0 = (nothing)
	1 0 = determine decimal point using current locale and overwrite current flags
	1 1 = determine decimal point using current locale and merge with current flags
*/
template<class char_t, std::floating_point fp_t>
constexpr size_t parse_fp(const char_t*p, fp_t& result, uint32_t flags = 15) noexcept
{
	if (flags & 16)
	{
		uint8_t new_flags;
		switch (localeconv()->decimal_point[0])
		{
		case '.':
			new_flags = 1;
			break;

		case ',':
			new_flags = 2;
			break;

		default:
			new_flags = 3;
			break;
		}

		if (flags & 32)
		{
			flags = (flags & ~0b11) | new_flags;
		}
		else
		{
			flags |= new_flags;
		}
	}

	const char_t* const p_begin = p;
	int exponent = 0;
	fp_t integral_part = 0, fractional_part = 0;
	bool negative = false, did_job = false;

	if (*p == '+') {}
	else if (*p == '-')
	{
		++p;
		negative = true;
	}

	if (iswdigit(*p))
	{
		did_job = true;
		do
		{
			integral_part = integral_part * 10 + *p - '0';
		} while (isdigit(*++p));
	}

	if ((*p == '.' && flags & 1) || (*p == ',' && flags & 2))
	{
		const char_t* const p_begin_fractional = p;

		while (iswdigit(*++p));

		const char_t* const p_end_fractional = p;

		if (p_begin_fractional + 1 != p_end_fractional)
		{
			did_job = true;
		}

#pragma warning(push)
#pragma warning(disable : 26451)
		while (p_begin_fractional < --p)
		{
			fractional_part += *p - '0';
			fractional_part /= 10;
		}
#pragma warning(pop)

		p = p_end_fractional;
	}

	if (did_job)
	{
		if ((*p == 'e' && flags & 4) || (*p == 'E' && flags & 8))
		{
			bool negative_exponent = false;
			++p;
			if (*p == '+')
			{
				++p;
			}
			else if (*p == '-')
			{
				negative_exponent = true;
				++p;
			}

			if (iswdigit(*p))
			{
				do
				{
					exponent = exponent * 10 + *p - '0';
				} while (iswdigit(*++p));

				if (negative_exponent)
				{
					exponent = -exponent;
				}
			}
			else
			{
				p -= 2;
			}
		}


		result = integral_part + fractional_part;
		if (exponent)
		{
			result *= std::pow(10, exponent);
		}
		if (negative)
		{
			result = -result;
		}

		return p - p_begin;
	}

	return 0;
}

template<std::floating_point float_t>
float_t map(float_t x, float_t from_begin, float_t from_end, float_t to_begin, float_t to_end)
{
	float_t df = from_end - from_begin;
	float_t dt = to_end - to_begin;
	return to_begin + (x - from_begin) * dt / df;
}





template<class fp_t>
bool is_inf_nan(const fp_t& x)
{
	using std::isinf;
	using std::isnan;
	return isinf(x) || isnan(x);
}

template<class fp_t>
bool is_inf_nan(const std::complex<fp_t>& x)
{
	using std::isinf;
	using std::isnan;
	return isinf(x.real()) || isinf(x.imag()) || isnan(x.real()) || isnan(x.imag());
}





_KSN_DETAIL_BEGIN

template<class fp_t, class callable_t, class... params_t>
constexpr bool newthon_method_general(callable_t func, fp_t& result, fp_t x, long double epsilon, long double h, size_t cycles_left, params_t&& ...params)
{
	using std::abs;

	fp_t temp, y;
	epsilon = fabsl(epsilon);

	while (cycles_left--)
	{
		if (is_inf_nan(x))
		{
			return false;
		}


		y = func(x, std::forward<params_t>(params)...);
		if (abs(y) <= epsilon)
		{
			result = x;
			return true;
		}

		if (x == fp_t(0)) x = fp_t(0.1);
		temp = x * fp_t(h);
		temp = (func((x + temp), std::forward<params_t>(params)...) - y) / (temp);

		if (fp_t(temp) == fp_t(0))
		{
			x *= (fp_t(1) + fp_t(h) * fp_t(10));
			continue;
		}


		temp = y / temp;

		if (abs((x - temp) - x) <= epsilon)
		{
			result = x - temp;
			return true;
		}
		x -= temp;
	}

	result = x;
	return false;
}

_KSN_DETAIL_END





template<class fp_t, class callable_t, class... params_t>
constexpr bool newthon_method(callable_t func, fp_t& result, fp_t x0 = 1, long double epsilon = 1e-4L, long double h = 1e-4L, size_t max_cycles = 50, params_t&& ...params)
{
	return detail::newthon_method_general(func, result, std::move(x0), epsilon, h, max_cycles, std::forward<params_t>(params)...);
}

template<std::floating_point fp_t, class callable_t, class... params_t>
constexpr bool newthon_method(callable_t func, fp_t& result, fp_t x0 = 1, long double epsilon = (long double)std::numeric_limits<fp_t>::epsilon() * 1000,
	long double h = (long double)std::numeric_limits<fp_t>::epsilon() * 1000, size_t max_cycles = 50, params_t&& ...params)
{
	return detail::newthon_method_general(func, result, std::move(x0), epsilon, h, max_cycles, std::forward<params_t>(params)...);
}

template<std::integral int_t, class callable_t, class... params_t>
constexpr bool newthon_method(callable_t&&, int_t&, int_t = 0, int_t = 0, int_t = 0, size_t = 0, params_t&& ...)
{
	return false;
}





template<class uint_t>
constexpr uint_t combinations_number(uint_t n, uint_t k)
{
	if (k > n) return 0; //Undefined
	if (k * 2 > n) k = n - k; //Due to symmetry
	if (k == 0) return 1; //There is only one way to chose 0 elements

	uint_t result = n;
	--n;
	for (uint_t i = 2; i <= k; ++i, --n)
	{
		result *= n;
		result /= i;
	}
	return result;
}





//Uses 2 calls to f(x)
template<class fp_t, class callable_t> requires(!std::is_integral_v<fp_t>)
fp_t differentiate1(callable_t&& f, const fp_t& x)
{
	constexpr static fp_t static_dx = ksn::root(std::numeric_limits<fp_t>::epsilon(), 2);

	fp_t dx = x ? static_dx * x : static_dx;
	dx += x;
	dx -= x;

	return (f(x + dx) - f(x - dx)) / (2 * dx);
}

//Uses 4 calls to f(x)
template<class fp_t, class callable_t> requires(!std::is_integral_v<fp_t>)
fp_t differentiate2(callable_t&& f, const fp_t& x)
{
	constexpr static fp_t static_dx = ksn::root(std::numeric_limits<fp_t>::epsilon(), 4);

	fp_t dx = x ? static_dx * x : static_dx;
	dx += x;
	dx -= x;

	fp_t d1f = f(x + dx) - f(x - dx);
	fp_t d2f = f(x + dx + dx) - f(x - dx - dx);

	return (d1f - d2f / 8) / (dx * 3 / 2);
}

//Uses 6 calls to f(x)
template<class fp_t, class callable_t> requires(!std::is_integral_v<fp_t>)
fp_t differentiate3(callable_t&& f, const fp_t& x)
{
	constexpr static fp_t static_dx = ksn::root(std::numeric_limits<fp_t>::epsilon(), 6);

	fp_t dx = x ? static_dx * x : static_dx;
	dx += x;
	dx -= x;

	fp_t d1f = f(x + dx) - f(x - dx);
	fp_t d2f = f(x + dx + dx) - f(x - dx - dx);
	fp_t d3f = f(x + dx + dx + dx) - f(x - dx - dx - dx);

	fp_t d13f = d1f - d3f / (3 * 3 * 3 * 3 * 3);
	fp_t d23f = d2f - d3f * fp_t(2 * 2 * 2 * 2 * 2) / (3 * 3 * 3 * 3 * 3);

	//deriving this was quite painful
	return (d13f - d23f / 5) / (4 * dx / 3);
}

//Uses 8 calls to f(x)
template<class fp_t, class callable_t> requires(!std::is_integral_v<fp_t>)
fp_t differentiate4(callable_t&& f, const fp_t& x)
{
	constexpr static fp_t static_dx = ksn::root(std::numeric_limits<fp_t>::epsilon(), 6);

	fp_t dx = x ? static_dx * x : static_dx;
	dx += x;
	dx -= x;

	fp_t d1f = f(x + dx) - f(x - dx);
	fp_t d2f = f(x + dx + dx) - f(x - dx - dx);
	fp_t d3f = f(x + dx + dx + dx) - f(x - dx - dx - dx);
	fp_t d4f = f(x + dx + dx + dx + dx) - f(x - dx - dx - dx - dx);

	//A shoutout to https://web.media.mit.edu/~crtaylor/calculator.html
	return (-3 * d4f + 32 * d3f - 168 * d2f + 672 * d1f) / (840 * dx);
}



_KSN_DETAIL_BEGIN

template<class fp_t, class callable>
fp_t limit_inf(callable&& f)
{
	return NAN;
}

template<class fp_t, class callable>
fp_t neighbourhood_approximator1(callable f, const fp_t& x, size_t N, const fp_t& dx, fp_t coefficient)
{
	fp_t sum = 0;
	fp_t step = dx + dx;

	for (size_t n = 1; n < N; ++n)
	{
		coefficient *= ptrdiff_t(n - N); //Negating occurs here
		coefficient /= (N + n + 1);
		sum += coefficient * (f(x + step) + f(x - step));
		step += dx;
	}

	return sum;
}

_KSN_DETAIL_END



/*
The function does it's best to find a function's limit at some point x
But results are gonna be totaly nonsence if you plug here some chaotic function
For example, lim x->0 of x*sin(e^(1/x)) will yield nonsence of 0.0004 whe the actual limit is 0
Consider using some tricks for corcer-case function
For example, use 1000*lim x->a (y(a)/1000) if the function oscilates a lot near the point x=a etc
*/
template<class fp_t, uint8_t custom_dx_order = 0>
fp_t limit(fp_t(*f)(fp_t), fp_t x)
{
	using std::ceil;
	using std::log;
	using std::abs;

	if (x != x) return x;
	if (x == INFINITY) return detail::limit_inf<fp_t>(f);
	if (x == -INFINITY) return detail::limit_inf<fp_t>([&](const fp_t& x) {return f(-x); });

	fp_t temp;
	//if (temp == temp) return temp;

	constexpr fp_t epsilon = std::numeric_limits<fp_t>::epsilon();

	fp_t dx;

	if constexpr (custom_dx_order == 0)
	{
		dx = epsilon * 100;
		if (x != 0) dx *= abs(x);
		dx = (x + dx) - x;

		static constexpr fp_t DX_STEP = fp_t(1.5);

		bool ok = true;
		while (true)
		{
			temp = f(x + dx);
			if (temp == temp)
			{
				temp += f(x - dx);
				if (temp == temp) break;
			}
			dx *= DX_STEP;
			if (dx >= 1)
			{
				ok = false;
				break;
			}
		};

		if (!ok) return NAN;
	}
	else
	{
		dx = pow(fp_t(0.1), custom_dx_order);
		if (x != 0) dx *= abs(x);
		dx = (x + dx) - x;
		temp = f(x + dx) + f(x - dx);
	}

	/*

	Let dn{f} denote n'th derivative of f
	Let A_k be 1/2 * [ f(x + k*dx) + f(x - k*dx) ]


	Using Taylor series axpansion at point x, one can get:
	A_k = f(x) + 1/(2!)*d2{f}*(k*dx)^2 + d4{f}*(k*dx)^4 / 4! ... up to some dN{f}
	Where f(x) and all it's derivatives are unknowns

	Select N such that dx^N < machine_epsilon => dx^n -> 0
	N > log base dx of epsilon
	N > ln(eps) / ln(dx)
	Equation (1) calculates nuber of equations needed (taking into account that we can only have even integer powers)

	All the equations are in form of A_k = f(...)
	Since we easily can compute all the A_k after we found some nice neighbourhood of the point [x - dx; x + dx] where f(x) can be computed

	Combine all the A_k into system of equations

	Note: initial aumented matrix looks like
	[   1   (1*dx)^2/(2!)   (1*dx)^4/(4!)   ...   |   A_1   ]
	[   1   (2*dx)^2/(2!)   (2*dx)^4/(4!)   ...   |   A_2   ]
	[   1   (3*dx)^2/(2!)   (3*dx)^4/(4!)   ...   |   A_3   ]
	[   ...


	If then one solves a couple of this systems (with different N) for f(x), one can prove the following:

	f(x) = 1/C(2N-1, N) * SUM from n=1 to N of [ (-1)^(n+1) * C(2N, N-n) * A_n ],

	which can be transformed info

	f(x) = SUM from n=1 to N of [ (-1)^(n+1) * A_n * Falling(N, n) / Rising(N+1, n) ],

	where Rising(N, n) and Falling(N, n) denote rising and falling N factorials respectively


	P.S.
	I compute f(x) by using the values of a function in some neighbourhood of x
	And it gets more and more and more precise as N -> infinity
	So yeah, I bet I have derived some kind of reverse of the Taylor series expansion
	¯\_(ツ)_/¯

	*/


	size_t N = (size_t)ceil(log(epsilon) / log(dx));
	if (N == 1)
		return temp / 2;
	else
		N = (N + 1) / 2 + 1 + bool(custom_dx_order);

	fp_t coefficient = fp_t(N) / (N + 1);
	return coefficient * temp + detail::neighbourhood_approximator1<fp_t>(f, x, N, dx, coefficient);
}


//Tries it's best to approximate the value of f(x) if it can not be directly evalueated in x but can be in [x-Ndx; x+Ndx]
template<class fp_t, class callable>
fp_t neighbourhood_approximator(callable f, const fp_t& x, size_t N, const fp_t& dx = fp_t(0.0001))
{
	fp_t c = fp_t(N);
	c /= N + 1;
	return c * (f(x + dx) + f(x - dx)) + detail::neighbourhood_approximator1(f, x, N, dx, c);
}


_KSN_END



#endif //_KSN_MATH_COMMON_HPP_
