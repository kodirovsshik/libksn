
#pragma once

#ifndef _KSN_MATH_COMMON_HPP_
#define _KSN_MATH_COMMON_HPP_


#include <ksn/ksn.hpp>
#include <ksn/math_complex.hpp>
#include <ksn/math_matrix.hpp>

#include <vector>
#include <stdexcept>
#include <concepts>
#include <complex>
#include <span>

#include <cmath>

#include <locale.h>





_KSN_BEGIN


/*
template<class fp1_t, class fp2_t>
std::vector<std::common_type_t<fp1_t, fp2_t>> polynomial_multiplication(const std::vector<fp2_t>& v1, const std::vector<fp2_t>& v2);

template<class fpy_t, class fpx_t>
std::vector<fpy_t> polynomial_interpolation(const std::vector<std::pair<fpx_t, fpy_t>>& points);

template<class fp_t>
std::vector<fp_t> polynomial_roots(std::vector<fp_t> coeffs)
{
	for (size_t i = 0; i < coeffs.size(); ++i)
	{
		if (coeffs[coeffs.size() - i - 1] == 0)
			coeffs.pop_back();
		else
			break;
	}
	using std::sqrt;
	using std::cbrt;
	using std::cos;
	using std::acos;
	using std::pow;

	if (coeffs.size() <= 1) return {};
	if (coeffs.size() == 2) return { -coeffs[0] / coeffs[1] };
	if (coeffs.size() == 3)
	{
		//Quadratic
		fp_t a = std::move(coeffs[2]);
		fp_t b = std::move(coeffs[1]);
		fp_t c = std::move(coeffs[0]);
		b = b / a / 2;
		c = c / a;
		fp_t D = b * b - c;
		if (c < 0) return {};
		D = sqrt(D);
		return { -b - D, -b + D };
	}
	if (coeffs.size() == 4)
	{
		//Cubic
		fp_t a = std::move(coeffs[3]);
		fp_t b = std::move(coeffs[2]);
		fp_t c = std::move(coeffs[1]);
		fp_t d = std::move(coeffs[0]);
		a = 1 / a;
		b *= a;
		c *= a;
		d *= a;
		b /= 3;
		fp_t b1 = -b;
		a = c - 3 * b * b;
		d = b * (2 * b * b - c) + d;
		d /= 2; //q
		a /= 3; //p
		fp_t D = d * d + a * a * a;
		if (D <= 0)
		{
			//Three real roots
			d /= a;
			a = sqrt(-a);
			fp_t k = 2 * a;
			a = 1 / a;
			fp_t t = acos(d * a);
			return
			{
				fp_t(k * cos((t) / 3) + b1),
				fp_t(k * cos((t - 2 * KSN_PI) / 3) + b1),
				fp_t(k * cos((t - 4 * KSN_PI) / 3) + b1)
			};
		}
		else
		{
			d = -d;
			D = sqrt(D);
			return { fp_t(cbrt(d + D) + cbrt(d - D) + b1) };
		}
	}
	if (coeffs.size() == 5)
	{
		//Quartic
		fp_t a = 1 / std::move(coeffs[4]);
		fp_t b = a * std::move(coeffs[3]);
		fp_t c = a * std::move(coeffs[2]);
		fp_t d = a * std::move(coeffs[1]);
		fp_t e = a * std::move(coeffs[0]);
		b /= 2;
		fp_t q = d + b * (b * b - c);
		b /= 2;
		fp_t b1 = b;
		fp_t p = c - 6 * b * b;
		fp_t r = e + b * (-d + b * (c - 3 * b));
		r *= -4;
		b = (r - p * p / 3);
		p /= -3;
		a = (p * (2 * p * p - r) - q * q - 3 * r * p) * -0.5;
		c = a * a + b * b * b;
		if (c < 0)
		{
			c = -c;
			a = 2 * pow(a * a + c, 1.0 / 6) * cos(1.0 / 3 * atan2(sqrt(c), a));
		}
		else
		{
			c = sqrt(c);
			a = cbrt(a + c) + cbrt(a - c);
		}
		a = (a - p) / 2;
		d = a * a - r;
		if (d < 0) return {};
		d = sqrt(d);
		b = a + d; //omega2
		a = a - d; //omega1
		if (d != 0)
			d = p / (c - b);
		else
			d = sqrt(c + b + 3 * p);
		d /= 2; //d <- z/2
		c = d * d;
		a = c - a;
		b = c - b;

		std::vector<fp_t> result;
		if (a > 0)
		{
			a = sqrt(a);
			result.push_back(-d - a);
			result.push_back(-d + a);
		}
		if (b > 0)
		{
			b = sqrt(b);
			result.push_back(d - b);
			result.push_back(d + b);
		}
		for (auto& x : result) x -= b1;
		return result;
	}
	throw 0;
	//TODO: implement the ultimate algorithm for solving algebraic equations
}
*/




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
Bit 2: allow use 'e' as exponent part begin
Bit 3: allow use 'E' as exponent part begin
Bits [4:5]:
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





template<class fp_t> requires(std::floating_point<fp_t>)
constexpr bool is_inf_nan(const fp_t& x) noexcept
{
	using std::isinf;
	using std::isnan;
	return x == INFINITY || x == -INFINITY || x != x;
}

template<class fp_t>
constexpr bool is_inf_nan(const std::complex<fp_t>& x) noexcept
{
	return is_inf_nan(x.real()) || is_inf_nan(x.imag());
}





_KSN_DETAIL_BEGIN

template<class fp_t, class callable_t, class... params_t>
constexpr bool newton_method_general(callable_t func, fp_t& result, fp_t x, long double epsilon, long double h, size_t cycles_left, params_t&& ...params)
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
constexpr bool newton_method(callable_t func, fp_t& result, fp_t x0 = 1, long double epsilon = 1e-4L, long double h = 1e-4L, size_t max_cycles = 50, params_t&& ...params)
{
	return detail::newton_method_general(func, result, std::move(x0), epsilon, h, max_cycles, std::forward<params_t>(params)...);
}

template<std::floating_point fp_t, class callable_t, class... params_t>
constexpr bool newton_method(callable_t func, fp_t& result, fp_t x0 = 1, long double epsilon = (long double)std::numeric_limits<fp_t>::epsilon() * 1000,
	long double h = (long double)std::numeric_limits<fp_t>::epsilon() * 1000, size_t max_cycles = 50, params_t&& ...params)
{
	return detail::newton_method_general(func, result, std::move(x0), epsilon, h, max_cycles, std::forward<params_t>(params)...);
}

template<std::integral int_t, class callable_t, class... params_t>
constexpr bool newton_method(callable_t&&, int_t&, int_t = 0, int_t = 0, int_t = 0, size_t = 0, params_t&& ...)
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
Consider using some tricks for corner-case function
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
	Let A_k := 1/2 * [ f(x + k*dx) + f(x - k*dx) ]


	Using Taylor series axpansion at point x, one can get:
	A_k = f(x) + d2{f}*(k*dx)^2 / 2! + d4{f}*(k*dx)^4 / 4! ... up to some dN{f}
	Where f(x) and all it's derivatives are unknowns

	Select N such that dx^N < machine_epsilon => dx^N -> 0
	N > log base dx of epsilon
	N > ln(eps) / ln(dx)
	From there, one cal calculate nuber of equations needed (taking into account that we can only have even integer powers)

	All the equations are in form of A_k = f(...)
	Since we easily can compute all the A_k after we found some nice neighbourhood of the point [x - dx; x + dx] where f(x) can be computed,

	Combine all the A_k into system of equations

	Note: initial augmented matrix looks like
	[   1   (1*dx)^2/(2!)   (1*dx)^4/(4!)   ...   |   A_1   ]
	[   1   (2*dx)^2/(2!)   (2*dx)^4/(4!)   ...   |   A_2   ]
	[   1   (3*dx)^2/(2!)   (3*dx)^4/(4!)   ...   |   A_3   ]
	[   ...


	If then one solves a couple of this systems (with different N) for f(x), one can prove the following:

	f(x) = 1/C(2N-1, N) * SUM from n=1 to N of [ (-1)^(n+1) * C(2N, N-n) * A_n ],

	which can be transformed info

	f(x) = SUM from n=1 to N of [ (-1)^(n+1) * A_n * Falling(N, n) / Rising(N+1, n) ],

	where Rising(N, n) and Falling(N, n) denote rising and falling N factorials respectively
	And C(n, k) is a binomial coefficient


	P.S.
	I compute f(x) by using the values of a function in some neighbourhood of x
	And it gets more and more and more precise as N -> infinity
	So yeah, I guess I have derived some kind of inverse of the Taylor series expansion
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
constexpr fp_t neighbourhood_approximator(callable f, const fp_t& x, size_t N, const fp_t& dx = fp_t(0.0001))
{
	fp_t c = fp_t(N);
	c /= N + 1;
	return c * (f(x + dx) + f(x - dx)) + detail::neighbourhood_approximator1(f, x, N, dx, c);
}





//x1 is for t = 0
//x2 is for t = 1
template<class fp_t>
constexpr fp_t interpolate_linear(fp_t x1, fp_t x2, fp_t t)
{
	return x1 + (x2 - x1) * t;
}


//x1 is for t = 0
//x2 is for t = 1/2
//x3 is for t = 1
template<class fp_t>
constexpr fp_t interpolate_quadratic(fp_t x1, fp_t x2, fp_t x3, fp_t t)
{
	constexpr auto matrix = ksn::matrix<3, 3, fp_t>
	{ {
		{ 1, 0, 0 },
		{ -3, 4, -1 },
		{ 2, -4, 2 }
	} };

	auto coeffs = matrix * ksn::vec<3, fp_t>{ x1, x2, x3 };

	return (coeffs[2] * t + coeffs[1]) * t + coeffs[0];
}


//x1 is for t = 0
//x2 is for t = 1/3
//x3 is for t = 2/3
//x4 is for t = 1
template<class fp_t>
constexpr fp_t interpolate_cubic(fp_t x1, fp_t x2, fp_t x3, fp_t x4, fp_t t)
{
	constexpr fp_t half = fp_t(1) / 2;

	constexpr auto matrix = ksn::matrix<4, 4, fp_t>
	{{
		{ 1, 0, 0, 0 },
		{ -11 * half , 9, -9 * half, 1 },
		{ 9, -45 * half, 18, -9 * half },
		{-9 * half, 27 * half, -27 * half, 9 * half },
	}};
	
	auto coeffs = matrix * ksn::vec<4, fp_t>{ x1, x2, x3, x4 };

	return ((coeffs[3] * t + coeffs[2]) * t + coeffs[1]) * t + coeffs[0];
}




_KSN_DETAIL_BEGIN

template<std::floating_point T>
constexpr T newton_method_get_epsilon() noexcept
{
	return std::numeric_limits<T>::epsilon();
}
template<class T>
constexpr T newton_method_get_epsilon() noexcept
{
	return 1e-8;
}
template<class T>
constexpr T newton_method_ct_sqrt(T x) noexcept
{
	T y = 1;
	for (int i = 0; i < 20; ++i)
	{
		y = T(0.5) * (y + x / y);
	}
	return y;
}

_KSN_DETAIL_END

template<class X, class Y = X>
struct newton_method_params
{
	Y y_threshold = detail::newton_method_get_epsilon<Y>() * Y(100);
	X derivative_step = detail::newton_method_ct_sqrt(X(10) * detail::newton_method_get_epsilon<X>());
	X coefficient = 1;
	X x_min = -INFINITY;
	X x_max = INFINITY;
	size_t max_iterations = 100;
};

_KSN_DETAIL_BEGIN

template<class X, class callee_t, size_t span_size = std::dynamic_extent, class Y = std::remove_cvref_t<std::invoke_result_t<callee_t, std::span<X, span_size>>>>
bool newton_method_multivariable(callee_t&& f, std::span<X, span_size> x, newton_method_params<X, Y>& params)
{
	size_t& iterations = params.max_iterations;
	const X& dx = params.derivative_step;

	using std::abs;
	using span_t = decltype(x);

	Y y, dy;
	X temp;

	const size_t n = x.size();

	std::vector<X> delta(x.size());
	std::vector<X> x1(x.size());
	std::vector<X> x2(x.size());

	auto x1span = span_t(x1);
	auto x2span = span_t(x2);

	auto calculate_partial_derivative = [&]
	(size_t index, const X& step)
	{
		auto get_derivative_value = [&]
		{
			return (f(x2span) - f(x1span)) / (step * 2);
		};
		temp = x1span[index];

		x1span[index] -= step;
		x2span[index] += step;

		dy = get_derivative_value();
		if (abs(dy) <= params.y_threshold)
		{
			x1span[index] = temp;
			x2span[index] += 9 * step;
			dy = get_derivative_value();
			if (abs(dy) <= params.y_threshold)
				return false;
			x[index] += 5 * step;
		}

		delta[index] = dy;
		return true;
	};

	y = f(x);
	if (abs(y) <= params.y_threshold)
		return true;

	while (iterations > 0)
	{
		--iterations;

		bool have_nonzero_derivative = false;

		std::copy(x.begin(), x.end(), x1.begin());
		std::copy(x.begin(), x.end(), x2.begin());

		for (size_t i = 0; i < x.size(); ++i)
		{
			auto& elem = x[i];
			X step = (elem == 0) ? dx : (elem * dx);

			step = (elem + step) - elem;


			if (calculate_partial_derivative(i, step))
			{
				have_nonzero_derivative = true;
				delta[i] = std::cbrt(y) / delta[i];
			}

			x1[i] = x2[i] = elem;
		}

		if (!have_nonzero_derivative)
			return false;

		for (size_t i = 0; i < x.size(); ++i)
			x[i] -= delta[i] / n * params.coefficient;

		y = f(x);
		if (abs(y) <= params.y_threshold)
			return true;
	}

	return false;
}

template<class X, class callee_t, size_t span_size = std::dynamic_extent, class Y = std::remove_cvref_t<std::invoke_result_t<callee_t, std::span<X, span_size>>>>
bool newton_method_multivariable(callee_t&& f, std::span<X, span_size> x)
{
	newton_method_params<X, Y> params;
	return newton_method_multivariable(f, x, params);
}


template<class X, class callee_t, size_t span_size = std::dynamic_extent, class Y = std::remove_cvref_t<std::invoke_result_t<callee_t, std::span<X, span_size>>>>
bool newton_method_multivariable_minimization(callee_t&& f, std::span<X, span_size> x, newton_method_params<X, Y>& params)
{
	size_t& iterations = params.max_iterations;
	const X& dx = params.derivative_step;

	using std::abs;
	using span_t = decltype(x);

	Y y, ymin;

	const size_t n = x.size();

	std::vector<X> delta(x.size());
	std::vector<X> x1(x.size());
	std::vector<X> x2(x.size());
	std::vector<X> xmin(x.size());

	auto x1span = span_t(x1);
	auto x2span = span_t(x2);

	auto calculate_partial_derivative = [&]
	(size_t index, const X& step)
	{
		x1span[index] -= step;
		x2span[index] += step;

		auto f0 = f(x);
		auto fn1 = f(x1span);
		auto fp1 = f(x2span);

		auto dy1 = (fn1 - f0) / step;
		auto dy2 = (fp1 - f0) / step;

		auto d2y = (dy1 + dy2);

		auto dy = (fp1 - fn1);
		auto d = dy / d2y;
		d += 0;
		if (d != d)
			d = 0;
		delta[index] = d;
		return d != 0;
		//return fabs(d) > params.y_threshold;
		//return true;
	};

	ymin = y = f(x);
	std::copy(x.begin(), x.end(), xmin.begin());
	//if (abs(y) <= params.y_threshold)
		//return true;

	while (iterations > 0)
	{
		--iterations;

		bool have_nonzero_derivative = false;

		std::copy(x.begin(), x.end(), x1.begin());
		std::copy(x.begin(), x.end(), x2.begin());

		for (size_t i = 0; i < x.size(); ++i)
		{
			auto& elem = x[i];
			X step = (elem == 0) ? dx : (elem * dx);

			step = (elem + step) - elem;


			if (calculate_partial_derivative(i, step))
			{
				have_nonzero_derivative = true;
				//delta[i] *= params.coefficient;
			}

			x1[i] = x2[i] = elem;
		}

		if (!have_nonzero_derivative)
		{
			std::copy(xmin.begin(), xmin.end(), x.begin());
			return true;
		}

		for (size_t i = 0; i < x.size(); ++i)
		{
			x[i] = std::clamp(x[i] - delta[i] / n * params.coefficient, params.x_min, params.x_max);
		}

		y = f(x);
		if (y < ymin)
		{
			std::copy(x.begin(), x.end(), xmin.begin());
			ymin = y;
		}
		//if (abs(y) <= params.y_threshold)
			//return true;
	}

	std::copy(xmin.begin(), xmin.end(), x.begin());
	return false;
}

_KSN_DETAIL_END


_KSN_END

#endif //_KSN_MATH_COMMON_HPP_
