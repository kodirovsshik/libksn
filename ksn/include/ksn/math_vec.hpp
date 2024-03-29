
#ifndef _KSN_MATH_VEC_HPP_
#define _KSN_MATH_VEC_HPP_


#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>

#include <type_traits>
#include <utility>
#include <initializer_list>

#include <cmath>


_KSN_BEGIN


template<class test_t, class template_t>
concept possibly_const = std::is_same_v<test_t, template_t> || std::is_same_v<test_t, const template_t>;


template<size_t N, class fp_t = float>
struct vec
{
private:

	static_assert(N != 0);

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

	using value_type = fp_t;
	using reference = fp_t&;
	using const_reference = const fp_t&;

	using iterator = fp_t*;
	using const_iterator = const fp_t*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	using difference_type = ptrdiff_t;
	using size_type = size_t;



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



#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

	constexpr vec() noexcept
	{
		for (auto& x : this->data) x = 0;
	}
	template<class ofp_t>
	constexpr vec(const vec<N, ofp_t>& other) noexcept
	{
		for (size_t i = 0; i < N; ++i)
			(*this)[i] = fp_t(other[i]);
	}
	template<class ofp_t>
	constexpr vec(vec<N, ofp_t>&& other) noexcept
	{
		for (size_t i = 0; i < N; ++i)
			(*this)[i] = fp_t(std::move(other[i]));
	}
	template<class ofp_t>
	constexpr vec(std::initializer_list<ofp_t> list) noexcept
	{
		for (size_t i = 0; i < std::min(list.size(), N); ++i)
			this->data[i] = (fp_t)list.begin()[i];

		for (size_t i = std::min(list.size(), N); i < N; ++i)
			this->data[i] = (fp_t)0;
	}
	template<possibly_const<fp_t> fp1_t, possibly_const<fp_t> fp2_t> requires(N == 2)
	constexpr vec(const std::pair<fp1_t, fp2_t>& pair)
		: data{ pair.first, pair.second }
	{
	}

#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif

	
	iterator begin() noexcept
	{
		return this->data;
	}
	const_iterator begin() const noexcept
	{
		return this->data;
	}

	iterator end() noexcept
	{
		return std::end(this->data);
	}
	const_iterator end() const noexcept
	{
		return std::end(this->data);
	}
	
	const_iterator cbegin() const noexcept
	{
		return this->data;
	}
	const_iterator cend() const noexcept
	{
		return std::end(this->data);
	}


	reverse_iterator rbegin() noexcept
	{
		return std::end(this->data) - 1;
	}
	const_reverse_iterator rbegin() const noexcept
	{
		return std::end(this->data) - 1;
	}

	reverse_iterator rend() noexcept
	{
		return this->data - 1;
	}
	const_reverse_iterator rend() const noexcept
	{
		return this->data - 1;
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return std::end(this->data) - 1;
	}
	const_reverse_iterator crend() const noexcept
	{
		return this->data - 1;
	}


	template<class ofp_t>
	constexpr common_vec(fp_t, ofp_t, N) operator+(const vec<N, ofp_t>& rhs) const noexcept
	{
		common_vec(fp_t, ofp_t, N) result;

		for (size_t i = 0; i < N; ++i)
			result[i] = (*this)[i] + rhs[i];

		return result;
	}
	template<class ofp_t>
	constexpr common_vec(fp_t, ofp_t, N) operator-(const vec<N, ofp_t>& rhs) const noexcept
	{
		common_vec(fp_t, ofp_t, N) result;
		
		for (size_t i = 0; i < N; ++i)
			result[i] = (*this)[i] - rhs[i];

		return result;
	}
	


	template<class ofp_t> requires(std::convertible_to<ofp_t, fp_t>)
	constexpr my_t friend operator*(const my_t& x, ofp_t y) noexcept
	{
		common_vec(fp_t, ofp_t, N) result;
		using common_fp_t = std::common_type_t<fp_t, ofp_t>;

		for (size_t i = 0; i < N; ++i)
		{
			result[i] = (common_fp_t)x[i] * (common_fp_t)y;
		}
		return result;
	}
	template<class ofp_t> requires(std::convertible_to<ofp_t, fp_t>)
	constexpr my_t friend operator/(const my_t& x, ofp_t y) noexcept
	{
		return x * (1 / fp_t(y));
	}

	template<class ofp_t> requires(std::convertible_to<ofp_t, fp_t>)
	constexpr my_t friend operator*(ofp_t x, const my_t& y) noexcept
	{
		return y * x;
	}


	template<class ofp_t>
	constexpr std::common_type_t<fp_t, ofp_t> operator*(const vec<N, ofp_t>& b) const noexcept
	{
		std::common_type_t<fp_t, ofp_t> result = 0;

		for (size_t i = 0; i < N; ++i)
			result += (*this)[i] * b[i];

		return result;
	}



	constexpr fp_t abs2() const noexcept
	{
		fp_t result = 0;
		for (const auto& x : this->data)
			result += x * x;
		return result;
	}

	constexpr fp_t abs() const noexcept
	{
		using std::hypot;
		using std::sqrt;

		if constexpr (N == 1) return this->data[0];
		if constexpr (N == 2) return hypot(this->data[0], this->data[1]);
		if constexpr (N == 3) return hypot(this->data[0], this->data[1], this->data[2]);

		return sqrt(this->abs2());
	}

	template<class A, class B> requires(N == 2 && std::convertible_to<fp_t, A> && std::convertible_to<fp_t, B>)
	constexpr operator std::pair<A, B>()
	{
		return std::pair<A, B>{ (A)this->data[0], (B)this->data[1] };
	}
	
	template<class = void> requires(N == 2)
	constexpr fp_t arg() const noexcept
	{
		using std::atan2;
		return atan2(this->data[1], this->data[0]);
	}

	constexpr my_t normalized() const noexcept
	{
		my_t copy(*this);
		copy.normalize();
		return copy;
	}
	constexpr my_t& normalize() noexcept
	{
		auto multiplier = this->abs();
		if (multiplier == 0)
			return *this;

		multiplier = 1 / multiplier;

		for (auto& x : this->data)
			x *= multiplier;

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


	constexpr void remove_zeros_signs() noexcept
	{
		//Maths: nooo zero can not have a sign!
		//IEEE 754: haha floats go +0 -0
		for (auto& zero : this->data)
		{
			if (zero == 0) zero = 0;
		}
	}


	constexpr bool operator==(const my_t& other) const noexcept
	{
		for (size_t i = N; i --> 0;)
		{
			if (this->data[i] != other.data[i])
				return false;
		}

		return true;
	}
	constexpr bool operator!=(const my_t& other) const noexcept
	{
		return !(*this == other);
	}

#undef common_vec
};



using vec2i = vec<2, int>;
using vec2f = vec<2, float>;
using vec2d = vec<2, double>;
using vec2l = vec<2, long double>;

using vec3i = vec<3, int>;
using vec3f = vec<3, float>;
using vec3d = vec<3, double>;
using vec3l = vec<3, long double>;

using vec4i = vec<4, int>;
using vec4f = vec<4, float>;
using vec4d = vec<4, double>;
using vec4l = vec<4, long double>;



template<size_t N, class fp_t>
constexpr void swap(ksn::vec<N, fp_t>& a, ksn::vec<N, fp_t>& b)
{
	using std::swap;
	for (size_t i = 0; i < N; ++i)
		std::swap(a[i], b[i]);
}

#define _ksn_define_vec_minmax(min_or_max)\
template<size_t N, class fp_t>\
constexpr auto min_or_max(const ksn::vec<N, fp_t>& a, const ksn::vec<N, fp_t>& b)\
{\
	using std::min_or_max;\
\
	ksn::vec<N, fp_t> result;\
	for (size_t i = 0; i < N; ++i)\
	{\
		result[i] = min_or_max(a[i], b[i]);\
	}\
\
	return result;\
}
_ksn_define_vec_minmax(min);
_ksn_define_vec_minmax(max);

#undef _ksn_define_vec_minmax


template<size_t N, class fp_t>
constexpr auto clamp(const vec<N, fp_t>& value, const vec<N, fp_t>& lower_bound, const vec<N, fp_t>& upper_bound)
{
	return max(min(value, upper_bound), lower_bound);
}


#define _ksn_define_vec_rounder(round_f) \
template<size_t N, class fp_t> \
constexpr auto round_f(vec<N, fp_t> v) \
{ \
	using std::round_f; \
	for (auto& x : v) \
		x = round_f(x); \
	return v; \
}
_ksn_define_vec_rounder(round);
_ksn_define_vec_rounder(floor);
_ksn_define_vec_rounder(ceil);

#undef _ksn_define_vec_rounder

_KSN_END


#endif //!_KSN_MATH_VEC_HPP_
