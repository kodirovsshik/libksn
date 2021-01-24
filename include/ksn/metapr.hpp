#ifndef _KSN_METAPR_HPP_
#define _KSN_METAPR_HPP_



#include <ksn/ksn.hpp>

#include <type_traits>



_KSN_BEGIN





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






_KSN_END



#endif //!_KSN_METAPR_HPP_
