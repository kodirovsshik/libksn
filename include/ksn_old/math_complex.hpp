#pragma once

#ifndef _KSN_COMPLEX_HPP_
#define _KSN_COMPLEX_HPP_


#include <ksn/ksn.hpp>
#include <vector>

_KSN_BEGIN



template<class T, class P0, class ...P1toN>
struct is_any_of : is_any_of<T, P1toN...> {};

template<class T, class ...P1toN>
struct is_any_of<T, T, P1toN...> : std::true_type {};

template<class T>
struct is_any_of<T, T> : std::true_type {};

template<class T, class U>
struct is_any_of<T, U> : std::false_type {};

template<class T, class ...Args>
constexpr bool is_any_of_v = is_any_of<T, Args...>::value;





template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;


struct complex
{
	bool is_polar;
	
	union
	{
		struct
		{
			long double x, y;
		};
		struct
		{
			long double l, r;
		};
	};

	struct as_polar {};
	struct as_algebraic {};



	_KSN_CONSTEXPR complex() noexcept;
	template<arithmetic T> complex(T real_part) noexcept;
	//complex(T real, T imag) noexcept;
	
	template<arithmetic T> complex(T real, T imag, as_algebraic = {}) noexcept;
	template<arithmetic T> complex(T length, T angle, as_polar) noexcept;

	complex(uninitialized_t) noexcept;

#ifdef _COMPLEX_
	template <typename float_type>
	complex(const std::complex<float_type>& other) noexcept;
#endif


	complex operator+() const noexcept;
	complex operator-() const noexcept;


	friend complex operator+(const complex& left, const complex& right) noexcept;
	friend complex operator-(const complex& left, const complex& right) noexcept;
	friend complex operator*(const complex& left, const complex& right) noexcept;
	friend complex operator/(const complex& left, const complex& right) noexcept;

	template<arithmetic T> friend complex operator+(const complex& left, T right) noexcept;
	template<arithmetic T> friend complex operator-(const complex& left, T right) noexcept;
	template<arithmetic T> friend complex operator*(const complex& left, T right) noexcept;
	template<arithmetic T> friend complex operator/(const complex& left, T right) noexcept;

	template<arithmetic T> friend complex operator+(T left, const complex& right) noexcept;
	template<arithmetic T> friend complex operator-(T left, const complex& right) noexcept;
	template<arithmetic T> friend complex operator*(T left, const complex& right) noexcept;
	template<arithmetic T> friend complex operator/(T left, const complex& right) noexcept;



	complex& operator+=(const complex& other) noexcept;
	complex& operator-=(const complex& other) noexcept;
	complex& operator*=(const complex& other) noexcept;
	complex& operator/=(const complex& other) noexcept;

	template<arithmetic T> complex& operator+=(T x) noexcept;
	template<arithmetic T> complex& operator-=(T x) noexcept;
	template<arithmetic T> complex& operator*=(T x) noexcept;
	template<arithmetic T> complex& operator/=(T x) noexcept;
	
	

	long double& real() noexcept;
	long double& imag() noexcept;

	long double real() const noexcept;
	long double imag() const noexcept;


	long double& length() noexcept;
	long double& argument() noexcept;

	long double length() const noexcept;
	long double argument() const noexcept;



	bool operator==(const complex& other) const noexcept;
	bool operator!=(const complex& other) const noexcept;

	template<arithmetic T> bool operator==(T x) const noexcept;
	template<arithmetic T> bool operator!=(T x) const noexcept;


	operator bool() const noexcept;
	template<arithmetic T> operator T() const noexcept;
};



template<arithmetic T> complex pow(const complex& num, T power);

complex pow(const complex& num, const complex& power);



complex sqrt(const complex& num);

complex cbrt(const complex& num);

complex root(const complex& num, uint64_t degree);

std::vector<complex> roots(const complex& num, size_t degree);



//Natural logarithm
complex log(const complex& num);
complex ln(const complex& num);

complex log2(const complex& num);

complex log10(const complex& num);
complex lg(const complex& num);

template<arithmetic T> complex logn(const complex& num, T base);



complex exp(const complex& num);

long double abs(const complex& x);



_KSN_END



#endif // !_KSN_COMPLEX_HPP_
