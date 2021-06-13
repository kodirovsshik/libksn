
#ifndef _KSN_MATH_VEC_HPP_
#define _KSN_MATH_VEC_HPP_


#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>

#include <type_traits>
#include <utility>
#include <initializer_list>

#include <cmath>


_KSN_BEGIN


template<size_t N, class fp_t = float>
struct vec
{
private:

	template<class...>
	struct is_vec : std::false_type
	{
	};

	template<size_t xN, class xfp_t>
	struct is_vec<vec<xN, xfp_t>> : std::true_type
	{
	};

	template<class... args>
	static constexpr bool is_vec_v = is_vec<args...>::value;


public:

	fp_t data[N];

	using value_type = fp_t;
	static constexpr size_t size = N;

	using my_t = vec<N, fp_t>;

#define common_vec(t1, t2, N) vec<N, std::common_type_t<t1, t2>>



	constexpr fp_t& operator[](size_t i) noexcept
	{
		return this->data[i];
	}
	constexpr const fp_t& operator[](size_t i) const noexcept
	{
		return this->data[i];
	}



	constexpr vec() noexcept
	{
		for (auto& x : this->data) x = 0;
	}
	template<class ofp_t>
	constexpr vec(const vec<N, ofp_t>& other) noexcept
	{
		for (size_t i = 0; i < N; ++i) (*this)[i] = fp_t(other[i]);
	}
	template<class ofp_t>
	constexpr vec(vec<N, ofp_t>&& other) noexcept
	{
		for (size_t i = 0; i < N; ++i) (*this)[i] = std::move(other[i]);
	}
	template<class ofp_t>
	constexpr vec(std::initializer_list<ofp_t> list) noexcept
	{
		for (size_t i = 0; i < std::min(list.size(), N); ++i)
		{
			this->data[i] = list.begin()[i];
		}
		for (size_t i = std::min(list.size(), N); i < N; ++i)
		{
			this->data[i] = 0;
		}
	}



	template<class fp_t1, class fp_t2, size_t N>
	constexpr common_vec(fp_t1, fp_t2, N) friend operator+(const vec<N, fp_t1>& a, const vec<N, fp_t2>& b) noexcept
	{
		common_vec(fp_t1, fp_t2, N) result;
		for (size_t i = 0; i < N; ++i)
		{
			result[i] = a[i] + b[i];
		}
		return result;
	}
	template<class fp_t1, class fp_t2, size_t N>
	constexpr common_vec(fp_t1, fp_t2, N) friend operator-(const vec<N, fp_t1>& a, const vec<N, fp_t2>& b) noexcept
	{
		common_vec(fp_t1, fp_t2, N) result;
		for (size_t i = 0; i < N; ++i)
		{
			result[i] = a[i] - b[i];
		}
		return result;
	}
	


	template<class ofp_t> requires (!is_vec_v<ofp_t>)
	constexpr my_t friend operator*(const my_t& x, ofp_t y) noexcept
	{
		common_vec(fp_t, ofp_t, N) result;
		for (size_t i = 0; i < N; ++i)
		{
			result[i] = x[i] * y;
		}
		return result;
	}
	template<class ofp_t> requires (!is_vec_v<ofp_t>)
	constexpr my_t friend operator/(const my_t& x, ofp_t y) noexcept
	{
		return x * (1 / y);
	}

	template<class ofp_t> requires (!is_vec_v<ofp_t>)
	constexpr my_t friend operator*(ofp_t x, const my_t& y) noexcept
	{
		return y * x;
	}
	template<class ofp_t> requires (!is_vec_v<ofp_t>)
	constexpr my_t friend operator/(ofp_t x, const my_t& y) noexcept
	{
		return y * (1 / x);
	}


	template<class fp_t1, class fp_t2, size_t N>
	constexpr std::common_type_t<fp_t1, fp_t2> friend operator*(const vec<N, fp_t1>& a, const vec<N, fp_t2>& b) noexcept
	{
		std::common_type_t<fp_t1, fp_t2> result = 0;
		for (size_t i = 0; i < N; ++i) result += a[i] * b[i];
		return result;
	}



	constexpr fp_t abs2() const noexcept
	{
		fp_t result = 0;
		for (const auto& x : this->data) result += x * x;
		return result;
	}

	constexpr fp_t abs() const noexcept
	{
		using std::hypot;
		using std::sqrt;

		if constexpr (N == 0) return 0; //wtf
		if constexpr (N == 1) return this->data[0];
		if constexpr (N == 2) return hypot(this->data[0], this->data[1]);
		if constexpr (N == 3) return hypot(this->data[0], this->data[1], this->data[2]);

		return sqrt(this->abs2());
	}

	constexpr std::enable_if_t<N == 2, fp_t> arg() const noexcept
	{
		using std::atan2;
		return atan2(this->data[1], this->data[0]);
	}

	constexpr my_t normalized() const noexcept
	{
		return *this / this->abs();
	}
	constexpr my_t& normalize() noexcept
	{
		fp_t multiplier = 1 / this->abs();
		for (auto& x : this->data) x /= multiplier;
		return *this;
	}



	template<class ofp_t>
	constexpr my_t& operator+=(const vec<N, ofp_t>& other) noexcept
	{
		for (size_t i = 0; i < N; ++i) this->data[i] += fp_t(other.data[i]);
		return *this;
	}
	template<class ofp_t>
	constexpr my_t& operator-=(const vec<N, ofp_t>& other) noexcept
	{
		for (size_t i = 0; i < N; ++i) this->data[i] -= fp_t(other.data[i]);
		return *this;
	}
	template<class ofp_t>
	constexpr my_t& operator*=(const ofp_t& other) noexcept
	{
		for (size_t i = 0; i < N; ++i) this->data[i] *= fp_t(other);
		return *this;
	}
	template<class ofp_t>
	constexpr my_t& operator/=(const ofp_t& other) noexcept
	{
		return (*this *= (1 / other));
	}


	void remove_zeros_signs() noexcept
	{
		//lol
		for (auto& zero : this->data)
		{
			if (zero == 0) zero = 0;
		}
	}

#undef common_vec
};



using vec2f = vec<2, float>;
using vec2d = vec<2, double>;
using vec2l = vec<2, long double>;

using vec3f = vec<3, float>;
using vec3d = vec<3, double>;
using vec3l = vec<3, long double>;

using vec4f = vec<4, float>;
using vec4d = vec<4, double>;
using vec4l = vec<4, long double>;



_KSN_END


#endif //!_KSN_MATH_VEC_HPP_