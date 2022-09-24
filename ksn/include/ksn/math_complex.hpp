
#pragma once

#ifndef _KSN_COMPLEX_HPP_
#define _KSN_COMPLEX_HPP_



#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>
#include <ksn/math_constexpr.hpp>
#include <ksn/math_constants.hpp>

#include <vector>
#include <type_traits>
#include <memory>
#include <complex>





_KSN_BEGIN

template<class fp_t>
struct complex
{

	static_assert(!std::is_reference_v<fp_t>, "");
	static_assert(!std::is_pointer_v<fp_t>, "");
	static_assert(!std::is_const_v<fp_t>, "");
	static_assert(!std::is_volatile_v<fp_t>, "");
	//I love this language. Where else can there be a const volatile void type?
	static_assert(!std::is_same_v<std::remove_cv_t<fp_t>, void>, "");
	static_assert(!std::is_integral_v<fp_t>, "");


private:

	using my_t = ksn::complex<fp_t>;


public:

	fp_t real, imag;

	using value_type = fp_t;


	constexpr complex() : real(0), imag(0) {}
	constexpr complex(fp_t x) : real(x), imag(0) {}
	constexpr complex(fp_t real, fp_t imag) : real(real), imag(imag) {}

	constexpr complex(const my_t& r) : real(r.real), imag(r.imag) {}

	template<class fp_t2>
	constexpr complex(const std::complex<fp_t2>& r) : real(r.real()), imag(r.imag()) {}

	constexpr my_t& operator=(const my_t& other) noexcept
	{
		this->real = other.real;
		this->imag = other.imag;
		return *this;
	}


	template<class fp_t2>
	constexpr bool operator==(const complex<fp_t2>& x) noexcept
	{
		return this->real == x.real && this->imag == x.imag;
	}
	template<class fp_t2>
	constexpr bool operator==(const fp_t2& x) noexcept
	{
		return this->real == x && this->imag == 0;
	}



	constexpr fp_t abs() const noexcept
	{
		using std::numeric_limits;
		using std::abs;
		using std::sqrt;

		fp_t ar = abs(real);
		if (this->imag == 0) return ar;

		fp_t ai = abs(imag);
		if (this->real == 0) return abs(this->imag);

		static constexpr fp_t upper_safe = ksn::sqrt(numeric_limits<fp_t>::max());
		if (ar > upper_safe)
		{
			ai /= ar;
			return ar * sqrt(1 + ai * ai);
		}
		else if (ai > upper_safe)
		{
			ar /= ai;
			return ai * sqrt(1 + ar * ar);
		}

		return sqrt(ai * ai + ar * ar);
	}
	constexpr fp_t abs2() const noexcept
	{
		return this->real * this->real + this->imag * this->imag;
	}

	constexpr my_t conj() const noexcept
	{
		return my_t{ this->real, -this->imag };
	}
	constexpr fp_t arg() const noexcept
	{
		using std::atan2;
		return atan2(this->imag, this->real);
	}


	constexpr my_t operator+() const noexcept
	{
		return *this;
	}
	constexpr my_t operator-() const noexcept
	{
		return my_t{ -this->real, -this->imag };
	}


	template<class fp_t2>
	constexpr my_t operator+(const complex<fp_t2>& other) const noexcept
	{
		return my_t{ this->real + other.real, this->imag + other.imag };
	}
	template<class fp_t2>
	constexpr my_t operator-(const complex<fp_t2>& other) const noexcept
	{
		return my_t{ this->real - other.real, this->imag - other.imag };
	}
	template<class fp_t2>
	constexpr my_t operator*(const complex<fp_t2>& other) const noexcept
	{
		return my_t{ this->real * other.real - this->imag * other.imag, this->real * other.imag + this->imag * other.real };
	}
	template<class fp_t2>
	constexpr my_t operator/(const complex<fp_t2>& other) const noexcept
	{
		fp_t divisor = 1 / other.abs2();
		return *this * other.conj() * divisor;
	}


	template<class fp_t2>
	constexpr my_t& operator+=(const complex<fp_t2>& other) noexcept
	{
		this->real += other.real;
		this->imag += other.imag;
		return *this;
	}
	template<class fp_t2>
	constexpr my_t& operator-=(const complex<fp_t2>& other) noexcept
	{
		this->real -= other.real;
		this->imag -= other.imag;
		return *this;
	}
	template<class fp_t2>
	constexpr my_t& operator*=(const complex<fp_t2>& other) noexcept
	{
		fp_t temp = this->real * other.real - this->imag * other.imag;
		this->imag = this->real * other.imag + this->imag * other.real;
		this->real = temp;
		return *this;
	}
	template<class fp_t2>
	constexpr my_t& operator/=(const complex<fp_t2>& other) noexcept
	{
		fp_t divisor = 1 / other.abs2();
		fp_t temp = this->real * other.real + this->imag * other.imag;
		this->imag = this->real * other.imag - this->imag * other.real;
		this->imag *= divisor;
		this->real = temp * divisor;
		return *this;
	}


	template<arithmetic fp_t2>
	constexpr my_t operator+(const fp_t2& x) const
	{
		return my_t(this->real + x, this->imag);
	}
	template<arithmetic fp_t2>
	constexpr my_t operator-(const fp_t2& x) const
	{
		return my_t(this->real - x, this->imag);
	}
	template<arithmetic fp_t2>
	constexpr my_t operator*(const fp_t2& x) const
	{
		return my_t(fp_t(this->real * x), fp_t(this->imag * x));
	}
	template<arithmetic fp_t2>
	constexpr my_t operator/(const fp_t2& x) const
	{
		return my_t(this->real / x, this->imag / x);
	}


	template<arithmetic fp_t2>
	constexpr friend my_t operator+(const fp_t2& a, const my_t& b)
	{
		return my_t{ b.real + a, b.imag };
	}
	template<arithmetic fp_t2>
	constexpr friend my_t operator-(const fp_t2& a, const my_t& b)
	{
		return my_t{ a - b.real, -b.imag };
	}
	template<arithmetic fp_t2>
	constexpr friend my_t operator*(const fp_t2& a, const my_t& b)
	{
		return my_t{ b.real * a, b.imag * a };
	}
	template<arithmetic fp_t2>
	constexpr friend my_t operator/(const fp_t2& a, const my_t& b)
	{
		return a / b.abs2() * b.conj();
	}


	template<arithmetic fp_t2>
	constexpr my_t& operator+=(const fp_t2& x)
	{
		this->real += x;
		return *this;
	}
	template<arithmetic fp_t2>
	constexpr my_t& operator-=(const fp_t2& x)
	{
		this->real -= x;
		return *this;
	}
	template<arithmetic fp_t2>
	constexpr my_t& operator*=(const fp_t2& x)
	{
		this->real *= x;
		this->imag *= x;
		return *this;
	}
	template<arithmetic fp_t2>
	constexpr my_t& operator/=(const fp_t2& x)
	{
		return *this *= fp_t(1) / x;
	}

};


template<class fp_t>
constexpr complex<fp_t> exp(const ksn::complex<fp_t>& x) noexcept
{
	ksn::complex<fp_t> result = 1, current = 1, temp[2];
	
	size_t c = 0;
	while (true)
	{
		current *= x;
		current /= ++c;
		
		temp[0] = temp[1] = result + current;
		if (is_inf_nan(temp[0])) return temp[0];

		temp[1] -= result; //Actual change
		if (temp[1] == 0) return result;
		
		result = temp[0];
	}
}


//Natural log
template<class fp_t>
constexpr complex<fp_t> log(const ksn::complex<fp_t>& x) noexcept
{
	using std::log;
	return complex<fp_t>{ fp_t(0.5)* log(x.abs2()), x.arg() };
}


template<class fp_t>
constexpr complex<fp_t> pow(const ksn::complex<fp_t>& a, const ksn::complex<fp_t>& b) noexcept
{
	return exp(b * log(a));
}
template<class fp_t, class fp_t2>
constexpr complex<fp_t> pow(const ksn::complex<fp_t>& a, const fp_t2& b) noexcept
{
	return exp(log(a) * b);
}
template<class fp_t>
constexpr complex<fp_t> pow(const fp_t& a, const ksn::complex<fp_t>& b) noexcept
{
	return exp(b * log(complex<fp_t>(a)));
}


template<class fp_t>
constexpr complex<fp_t> sqrt(const ksn::complex<fp_t>& x)
{
	return (x.abs2() == 0) ? 0 : pow(x, fp_t(1) / 2);
}


template<class fp_t>
constexpr complex<fp_t> cbrt(const ksn::complex<fp_t>& x)
{
	using std::sin; using std::cos; using std::pow;
	fp_t len = pow(x.abs2(), 1.0 / 6);
	fp_t angle = x.arg();
	if (angle > KSN_PI / 2) angle += 2 * KSN_PI;
	else if (angle < -KSN_PI / 2) angle -= 2 * KSN_PI;
	angle /= 3;
	return { len * cos(angle), len * sin(angle) };
}

template<class fp_t>
void roots(const complex<fp_t>& x, size_t degree, complex<fp_t>* result)
{
	using std::pow;
	using std::cos;
	using std::sin;

	if (degree == 0) return;

	complex<fp_t> first = root(x, degree);
	*result = first;
	if (degree == 1) return;

	complex<fp_t> rotator(cos(2 * KSN_PId / degree), sin(2 * KSN_PId / degree));
	for (size_t i = 1; i < degree; ++i)
	{
		first *= rotator;
		*++result = first;
	}
}


template<class fp_t>
constexpr bool is_inf_nan(const complex<fp_t>& x) noexcept
{
	return x.real == INFINITY || x.real == -INFINITY || x.real != x.real || x.imag == INFINITY || x.imag == -INFINITY || x.imag != x.imag;
}

_KSN_END




#endif // !_KSN_COMPLEX_HPP_
