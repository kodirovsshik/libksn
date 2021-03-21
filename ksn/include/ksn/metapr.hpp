#ifndef _KSN_METAPR_HPP_
#define _KSN_METAPR_HPP_



#include <ksn/ksn.hpp>

#include <type_traits>



_KSN_BEGIN

_KSN_DETAIL_BEGIN


template<size_t x, size_t c = 0>
struct ct_log2
{
	static constexpr size_t value = ct_log2<x / 2, c + 1>::value;
};
template<size_t c>
struct ct_log2<1, c>
{
	static constexpr size_t value = c;
};
template<size_t c>
struct ct_log2<0, c>
{
};


_KSN_DETAIL_END





template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;







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





template<class A, class B>
struct is_same_to_cv
	: std::bool_constant< std::is_same_v<std::remove_cv_t<A>, std::remove_cv_t<B>> >
{
};

template<class A, class B>
constexpr bool is_same_to_cv_v = is_same_to_cv<A, B>::value;

template<class T, class U>
concept same_to_cv = is_same_to_cv_v<T, U>;





template<class A, class B>
struct is_compatible
	: std::false_type
{	
};

template<class A>
struct is_compatible<A, A>
	: std::true_type
{
};

template<class A, ksn::same_to_cv<A> B>
struct is_compatible<A, B>
	: std::true_type
{
};


template<class A, class B>
static constexpr bool is_compatible_v = is_compatible<A, B>::value;





template<class WHAT, class TO>
concept universal_reference = ksn::is_same_to_cv_v<std::remove_reference_t<TO>, std::remove_reference_t<WHAT>>;





template<size_t N>
static constexpr size_t ct_log2 = detail::ct_log2<N, 0>::value;



_KSN_END



#endif //!_KSN_METAPR_HPP_
