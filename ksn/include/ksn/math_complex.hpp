
#pragma once

#ifndef _KSN_COMPLEX_HPP_
#define _KSN_COMPLEX_HPP_



#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>
//#include <ksn/math_common.hpp>
#include <ksn/math_constants.hpp>

#include <vector>
#include <type_traits>
#include <memory>


_KSN_BEGIN





template<class T>
struct complex
{

	static_assert(!std::is_reference_v<T>, "");
	static_assert(!std::is_pointer_v<T>, "");
	static_assert(!std::is_const_v<T>, "");
	static_assert(!std::is_volatile_v<T>, "");
	//I love this language. Where else can there be const volatile void type?
	static_assert(!std::is_same_v<std::remove_cv_t<T>, void>, "");
	static_assert(!std::is_integral_v<T>, "");



	union
	{
		struct
		{
			std::aligned_storage_t<sizeof(T), alignof(T)> x, y; //x + y*i
		};
		struct
		{
			std::aligned_storage_t<sizeof(T), alignof(T)> n, a; //n^1/2 * e^(i*a)
			//'n' for norm, 'a' for angle
		};
	};

	bool is_polar;



private:

	template<class T1>
	static const T1& x_of(const complex<T1>& obj) { return *reinterpret_cast<const T1*>(&obj.x); }
	template<class T1>
	static const T1& y_of(const complex<T1>& obj) { return *reinterpret_cast<const T1*>(&obj.y); }
	template<class T1>
	static const T1& n_of(const complex<T1>& obj) { return *reinterpret_cast<const T1*>(&obj.n); }
	template<class T1>
	static const T1& a_of(const complex<T1>& obj) { return *reinterpret_cast<const T1*>(&obj.a); }

	template<class T1>
	static T1& x_of(complex<T1>& obj) { return *reinterpret_cast<T1*>(&obj.x); }
	template<class T1>
	static T1& y_of(complex<T1>& obj) { return *reinterpret_cast<T1*>(&obj.y); }
	template<class T1>
	static T1& n_of(complex<T1>& obj) { return *reinterpret_cast<T1*>(&obj.n); }
	template<class T1>
	static T1& a_of(complex<T1>& obj) { return *reinterpret_cast<T1*>(&obj.a); }

#define this_x x_of(*this)
#define this_y y_of(*this)
#define this_n n_of(*this)
#define this_a a_of(*this)



public:

	using my_t = ksn::complex<T>;



	constexpr void ensure_algebraic() noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>&& std::is_nothrow_constructible_v<T> && noexcept(sin(std::declval<T>())) && noexcept(cos(std::declval<T>())) && noexcept(sqrt(std::declval<T>())))
	{
		if (this->is_polar)
		{
			T length = std::move(this_n);
			length = sqrt(length);

			T angle = std::move(this_a);
			T y = sin(angle) * length;
			length *= cos(angle); // Now stores x coord

			std::destroy_at(&this_n);
			std::destroy_at(&this_a);
			
			std::construct_at(&this_x, std::move(length));
			std::construct_at(&this_y, std::move(y));

			this->is_polar = false;
		}
	}

	constexpr void ensure_polar() noexcept(std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_destructible_v<T>&& std::is_nothrow_constructible_v<T> && noexcept(std::declval<T>() * std::declval<T>()) && noexcept(atan2(std::declval<T>(), std::declval<T>())))
	{
		if (this->is_polar == false)
		{
			T norm = this_x * this_x + this_y * this_y;
			T angle = atan2(this_y, this_x);

			std::destroy_at(&this_x);
			std::destroy_at(&this_y);

			std::construct_at(&this_n, std::move(norm));
			std::construct_at(&this_a, std::move(angle));

			this->is_polar = true;
		}
	}



	constexpr complex() noexcept(std::is_nothrow_default_constructible_v<T>)
		: is_polar(true)
	{
		std::construct_at(&this_n);
		std::construct_at(&this_a);
	}

	template<class T1>
	constexpr complex(T1 real_part) noexcept(std::is_nothrow_default_constructible_v<T> && std::is_nothrow_move_constructible_v<T>)
		 : is_polar(false)
	{
		std::construct_at(&this_x, std::forward<T>(real_part));
		std::construct_at(&this_y);
	}

	template<class T1, class T2>
	constexpr complex(T1&& real, T2&& imag) noexcept : is_polar(false)
	{
		std::construct_at(&this_x, std::forward<T>(real));
		std::construct_at(&this_y, std::forward<T>(imag));
	}

//I hope it will compile without <complex> header due to instantiation principles
#if false && (defined _COMPLEX_ || defined _GLIBCXX_COMPLEX_ || defined KSN_COMPLEX_STD)
	template <typename float_type>
	constexpr complex(const std::complex<float_type>& other) noexcept : is_polar(true)
	{
		std::construct_at(&this_n, std::norm(other));
		std::construct_at(&this_a, std::arg(other));
	}
#endif



	template<class T1>
	constexpr complex(const complex<T1>& other) noexcept : is_polar(other.is_polar)
	{
		if (!other.is_polar)
		{
			std::construct_at(&this_x, std::forward<T1>(x_of(other)));
			std::construct_at(&this_y, std::forward<T1>(y_of(other)));
		}
		else
		{
			std::construct_at(&this_n, std::forward<T1>(r_of(other)));
			std::construct_at(&this_a, std::forward<T1>(a_of(other)));
		}
	}

	template<class T1>
	constexpr complex(complex<T1>&& other) noexcept : is_polar(other.is_polar)
	{
		if (!other.is_polar)
		{
			std::construct_at(&this_x, std::move(x_of(other)));
			std::construct_at(&this_y, std::move(y_of(other)));
		}
		else
		{
			std::construct_at(&this_n, std::move(n_of(other)));
			std::construct_at(&this_a, std::move(a_of(other)));
		}
	}



#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif
	constexpr complex(uninitialized_t) noexcept {}
#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif


	template<class T1, class T2>
	static complex<T> from_polar_with_norm(T1&& norm, T2&& angle) noexcept(std::is_nothrow_move_constructible_v<T1> && std::is_nothrow_move_constructible_v<T2>)
	{
		ksn::complex<T> result(uninitialized);

		const void* p = &result;
		const size_t s = sizeof(result);
		const void* pe = &result + 1;

		const void* p1 = &n_of(result);
		const void* p2 = &a_of(result);

		result.is_polar = true;
		std::construct_at(&n_of(result), std::forward<T1>(norm));
		std::construct_at(&a_of(result), std::forward<T2>(angle));

		return result;
	}

	template<class T1, class T2>
	static complex<T> from_polar(T1&& length, T2&& angle) noexcept(std::is_nothrow_move_constructible_v<T1> && std::is_nothrow_move_constructible_v<T2>)
	{
		ksn::complex<T> result(ksn::uninitialized);

		result.is_polar = true;
		std::construct_at(&n_of(result), length * length);
		std::construct_at(&a_of(result), std::forward<T2>(angle));

		return result;
	}



	constexpr complex<T> operator+() const noexcept
	{
		return *this;
	}
	constexpr complex<T> operator-() const noexcept
	{
		my_t result(*this);

		if (this->is_polar)
		{
			a_of(result) += KSN_PI / 2;
		}
		else
		{
			x_of(result) *= -1;
			y_of(result) *= -1;
		}

		return result;
	}

	template<class T1, class T2>
	constexpr friend complex operator+(const complex<T1>& left, const complex<T2>& right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator+(const complex<T1>& left, complex<T2>& right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator-(const complex<T1>& left, const complex<T2>& right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator*(const complex<T1>& left, const complex<T2>& right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator/(const complex<T1>& left, const complex<T2>& right) noexcept;

	template<class T1, class T2>
	constexpr friend complex operator+(const complex<T1>& left, T2 right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator-(const complex<T1>& left, T2 right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator*(const complex<T1>& left, T2 right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator/(const complex<T1>& left, T2 right) noexcept;

	template<class T1, class T2>
	constexpr friend complex operator+(T1 left, const complex<T2>& right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator-(T1 left, const complex<T2>& right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator*(T1 left, const complex<T2>& right) noexcept;
	template<class T1, class T2>
	constexpr friend complex operator/(T1 left, const complex<T2>& right) noexcept;



	template<class T1>
	constexpr complex<T>& operator+=(const complex<T1>& other) noexcept;
	template<class T1>
	constexpr complex<T>& operator-=(const complex<T1>& other) noexcept;
	template<class T1>
	constexpr complex<T>& operator*=(const complex<T1>& other) noexcept;
	template<class T1>
	constexpr complex<T>& operator/=(const complex<T1>& other) noexcept;

	template<class T1>
	constexpr complex<T>& operator+=(T1 x) noexcept;
	template<class T1>
	constexpr complex<T>& operator-=(T1 x) noexcept;
	template<class T1>
	constexpr complex<T>& operator*=(T1 x) noexcept;
	template<class T1>
	constexpr complex<T>& operator/=(T1 x) noexcept;



	constexpr T real() const noexcept
	{
		if (this->is_polar)
		{
			return sqrt(this_n) * cos(this_a);
		}
		return this_x;
	}
	constexpr T imag() const noexcept
	{
		if (this->is_polar)
		{
			return sqrt(this_n) * sin(this_a);
		}
		return this_y;
	}

	constexpr T length() noexcept
	{
		this->ensure_polar();
		return sqrt(this_n);
	}
	constexpr T norm() const noexcept
	{
		this->ensure_polar();
		return this_n;
	}
	constexpr T argument() const noexcept
	{
		this->ensure_polar();
		return this_a;
	}



	constexpr T& real_ref() noexcept
	{
		this->ensure_algebraic();
		return this_x;
	}
	constexpr T& imag_ref() noexcept
	{
		this->ensure_algebraic();
		return this_x;
	}

	constexpr T& norm_ref() noexcept
	{
		this->ensure_polar();
		return this_n;
	}
	constexpr T& argument_ref() noexcept
	{
		this->ensure_polar();
		return this_a;
	}



	template<class T1>
	constexpr bool operator==(const complex<T1>& other) const noexcept
	{
		if (this->is_polar == false && other.is_polar == false)
			return this->real() == other.real() && this->imag() == other.imag();
		else
			return this->real2() == other.real2() && this->imag2() == other.imag2();
	}
	template<class T1>
	constexpr bool operator!=(const complex<T1>& other) const noexcept
	{
		if (this->is_polar == false && other.is_polar == false)
			return this->real() != other.real() || this->imag() != other.imag();
		else
			return this->real2() != other.real2() || this->imag2() != other.imag2();
	}
	
	template<std::integral T1>
	constexpr bool operator==(T1 x) const noexcept;
	template<std::floating_point T1>
	constexpr bool operator==(T1 x) const noexcept;

	template<std::integral T1>
	constexpr bool operator!=(T1 x) const noexcept;
	template<std::floating_point T1>
	constexpr bool operator!=(T1 x) const noexcept;

	constexpr bool operator==(const T&) const noexcept;


	constexpr operator bool() const noexcept
	{
		if (this->is_polar)
		{
			return this_n != 0;
		}
		else
		{
			return this_x != 0 && this_y != 0;
		}
	}

#undef this_x
#undef this_y
#undef this_n
#undef this_a
};

/*

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

*/

_KSN_END



#endif // !_KSN_COMPLEX_HPP_
