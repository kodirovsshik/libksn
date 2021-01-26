#pragma once

#ifndef _KSN_MATH_COMMON_HPP_
#define _KSN_MATH_COMMON_HPP_


#include <ksn/ksn.hpp>
#include <ksn/math_complex.hpp>

#include <vector>





#define KSN_PI					3.141592653589793238462643383279502884L
#define KSN_PId				3.14159265358979323
#define KSN_PIf					3.14159265f

#define KSN_E					2.718281828459045235360287471352662497L
#define KSN_Ed					2.71828182845904523
#define KSN_Ef					2.71828182f

#define KSN_PHI				1.618033988749894848204586834365638117L
#define KSN_PHId				1.61803398874989484
#define KSN_PHIf				1.61803398f

#define KSN_GAMMA			0.577215664901532860606512090082402431L
#define KSN_GAMMAd		0.57721566490153286
#define KSN_GAMMAf			0.5772156649f

//#define KSN_I				sqrt(-1)





_KSN_BEGIN



size_t log2_8(uint8_t x);
size_t log2_16(uint16_t x);
size_t log2_32(uint32_t x);
size_t log2_64(uint64_t x);



std::vector<double> polynomial_multiplication(const std::vector<double>& v1, const std::vector<double>& v2);

std::vector<double> polynomial_interpolation(const std::vector<std::pair<double, double>>& points);




template<typename T>
void DFT(const T* in, ksn::complex* out, size_t n)
{
#ifndef _KSN_NO_DFT_INT64_WARNING
	static_assert(sizeof(long double) == 16 || !(std::is_same_v<T, uint64_t> | std::is_same_v<T, int64_t>),
		"Warning: starting from 2^52 not every signle integer value can be represented with double precision FP type. "
		"Keep this in mind when calling a DFT of a int64 array. "
		"To suppress this warning, define _KSN_NO_DFT_INT64_WARNING before any <ksn/math*.hpp> inclusion. "
		"You see this warning because sizeof(long double) on your system is not 16 so you better know what you are doing"
		);
#endif

	const long double angle_coefficient = 2 * KSN_PI / (long double)(n);

	for (size_t i = 0; i < n; ++i)
	{
		ksn::complex current = 0;

		for (size_t j = 0; j < n; ++j)
		{
			long double angle = (long double)(uint64_t(i) * j) * angle_coefficient;
			current += in[j] * ksn::complex(cos(angle), sin(angle));
		}

		out[i] = current;
	}
}

template<typename T>
void IDFT(const ksn::complex* in, T* out, size_t n)
{
#ifndef _KSN_NO_DFT_INT64_WARNING
	static_assert(sizeof(long double) == 16 || !(std::is_same_v<T, uint64_t> | std::is_same_v<T, int64_t>),
		"Warning: starting from 2^52 not every signle integer value can be represented with double precision FP type. "
		"Keep this in mind when calling an IDFT of a int64 array. "
		"To suppress this warning, define _KSN_NO_DFT_INT64_WARNING before any <ksn/math*.hpp> inclusion. "
		"You see this warning because sizeof(long double) on your system is not 16 so you better know what you are doing"
		);
#endif
	const double angle_coefficient = -2 * KSN_PI / (long double)(n);

	for (size_t i = 0; i < n; ++i)
	{
		ksn::complex current = 0;

		for (size_t j = 0; j < n; ++j)
		{
			long double angle = angle_coefficient * i * j; //-V113
			current += in[j] * ksn::complex(cos(angle), sin(angle));
		}

		out[i] = T((current / (long double)(n)).data[0] + 0.5L);
	}
}



//I do need to take the input by copy
std::vector<int64_t> solve_integer_polynomial(std::vector<int64_t> coefficients);


uint64_t isqrt(uint64_t n);
uint32_t isqrt(uint32_t n);
uint16_t isqrt(uint16_t n);
uint8_t isqrt(uint8_t n);



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
int_type fp_int_cast(long double x, unsigned precision = LDBL_DECIMAL_DIG - 2)
{
#undef min
#undef max

	if _KSN_CONSTEXPR_CONDITION(!std::is_integral_v<int_type>)
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
		sprintf_s(buffer, 1024, "%.*Lg", precision, x);

		int read;

		using namespace std::string_literals;

		int_type result;
		sscanf_s(buffer, detail::_fp_int_cast_printf_format_helper<int_type>, &result, &read);

		if (buffer[read] != '\0')
		{
			return int_type(x);
		}

		return result;
	}
}



int solve_quadratic(long double a, long double b, long double c, long double(*p_roots)[2]);

int solve_quadratic(long double a, long double b, long double c, ksn::complex(*p_roots)[2]);

int solve_quadratic(const ksn::complex& a, const ksn::complex& b, const ksn::complex& c, ksn::complex(*p_roots)[2]);


long double solve_cubic_principal(long double a, long double b, long double c, long double d);

int solve_cubic(long double a, long double b, long double c, long double d, long double(*p_roots)[3]);

int solve_qartic(long double a, long double b, long double c, long double d, long double e, long double(*p_roots)[4]);



/*
 (m)
S
  n
Stigling number of the first kind
*/
template<typename return_type = long long>
return_type stirling_s1(size_t n, size_t m)
{
	if _KSN_CONSTEXPR_CONDITION(!(std::is_integral_v<return_type> || std::is_floating_point_v<return_type>) || std::is_same_v<return_type, bool>)
	{
		_KSN_RAISE(_STD invalid_argument("Incorrect return type passed to stirling_s1"));
	}
	else
	{
		if (m > n)
		{
			throw _STD invalid_argument("stirling_s1: m must be <= n");
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



long double sign(long double x);

bool isnan(const ksn::complex& x);

bool isinf(const ksn::complex& x);



_KSN_DETAIL_BEGIN

template<typename value_type>
void _newthon_method_helper_adjust_step(value_type& step, const value_type& x, int64_t k)
{
	step = k * LDBL_EPSILON * sign(x);
}

_KSN_DETAIL_END



template<typename value_type, typename function_t, typename derivative_t = value_type(*)(value_type, ...), class ... parameters_t>
bool newthon_method(value_type* result, function_t p_function, derivative_t p_derivative_or_nullptr, long double precision_x = 1e-8, long double precision_y = 1e-8, value_type start = 0, size_t max_iterations = 1000, parameters_t&& ...parameters)
{
	using fp = value_type;
	fp x = start;

	fp previous_step = 0;

	size_t i = 0;
	while (i++ < max_iterations)
	{
		fp y = p_function(x, std::forward<parameters_t &&...>(parameters...));
		if (compare_equal_with_precision<const fp&>(y, 0, precision_y))
		{
			*result = x;
			return true;
		}

		fp derivative;
		if (p_derivative_or_nullptr != nullptr)
		{
			derivative = p_derivative_or_nullptr(x, std::forward<parameters_t &&...>(parameters...));
		}
		else
		{
			derivative = 0;
		}
		int64_t k = 100000;

		if (derivative != 0) {}
		else if (x != 0)
		{
			do
			{
				fp step;
				detail::_newthon_method_helper_adjust_step(step, x, k);
				fp y1 = p_function(x * (1 + step), std::forward<parameters_t &&>(parameters)...);
				fp y2 = p_function(x * (1 - step), std::forward<parameters_t &&>(parameters)...);

				if (isnan(y1) || isinf(y1))
				{
					if (isnan(y2) || isinf(y2))
					{
						continue;
					}
					else
					{
						derivative = (y - y2) / (k * LDBL_EPSILON);
					}
				}
				else
				{
					derivative = (y1 - y) / (k * LDBL_EPSILON);
				}

				k *= 10;

			} while (k < 1000000000000 && derivative == 0);
		}
		else
		{
			do
			{
				value_type step;
				detail::_newthon_method_helper_adjust_step(step, x, k);
				fp y1 = p_function(x * (1 + step), std::forward<parameters_t &&>(parameters)...);
				fp y2 = p_function(x * (1 - step), std::forward<parameters_t &&>(parameters)...);

				if (isnan(y1) || isinf(y1))
				{
					if (isnan(y2) || isinf(y2))
					{
						continue;
					}
					else
					{
						derivative = (y - y2) / (k * LDBL_EPSILON);
					}
				}
				else
				{
					derivative = (y1 - y) / (k * LDBL_EPSILON);
				}

				k *= 10;

			} while (k < 1000000000000 && derivative == 0);
		}

		if (derivative == 0)
		{
			return false;
		}

		fp step = y / derivative;

		if (step == -previous_step)
		{
			step /= 2;
		}
		previous_step = step;

		if (step - x == x)
		{
			*result = x;
			return true;
		}

		x -= step;

		if (compare_equal_with_precision<const fp&>(step, 0, precision_x))
		{
			*result = x;
			return true;
		}
	}

	return false;
}



long double lambert_W0(long double x);

long double lambert_W_n1(long double x);

complex lambert_W(complex z, int n = 0);



/*
Flags:
Bit 0: allow '.' use be used as decimal point
Bit 1: allow ',' use be used as decimal point
Bit 2: allow use 'e' as exponent part begin ( mantiss*10^exponent )
Bit 3: allow use 'E' as exponent part begin ( mantiss*10^exponent )
Bit [4:5]:
	0 0 = (nothing)
	1 0 = determine decimal point useng locale and overwrite current flags
	1 1 = determine decimal point useng locale and merge with current flags
*/
template<class CharT>
size_t parse_fp(const CharT *p, long double &result, uint32_t flags = 15) noexcept
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

	const CharT* const p_begin = p;
	int exponent = 0;
	long double integral_part = 0, fractional_part = 0;
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
		const CharT* const p_begin_fractional = p;

		while (iswdigit(*++p));

		const CharT* const p_end_fractional = p;

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
			result *= powl(10, exponent);
		}
		if (negative)
		{
			result = -result;
		}

		return p - p_begin;
	}

	return 0;
}



_KSN_END


#endif //_KSN_MATH_COMMON_HPP_
