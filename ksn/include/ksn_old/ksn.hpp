
#ifndef _KSN_HPP_
#define _KSN_HPP_

#ifdef __cplusplus





#include <stdint.h>
//#include <type_traits>
#include <float.h>
#include <limits.h>

#undef _KSN
#undef _KSN_BEGIN
#undef _KSN_END
#undef _KSN_DETAIL_BEGIN
#undef _KSN_DETAIL_END

#define _KSN ::ksn::
#define _KSN_BEGIN namespace ksn{
#define _KSN_END }
#define _KSN_DETAIL_BEGIN namespace detail{
#define _KSN_DETAIL_END }





#ifdef _MSVC_LANG

	#define _KSN_COMPILER_MSVC

#elif (defined(__GNUC__) || defined(__GNUG__)) && !defined __clang__

	#define _KSN_COMPILER_GCC

#elif defined __clang__

	#define _KSN_COMPILER_CLANG

#else

	#define _KSN_COMPILER_UNKNOWN

#endif

#if defined __GNUC__ || defined __GNUG__
	#define _KSN_COMPILER_GNU
#endif





#ifdef _KSN_COMPILER_MSVC

	#define _KSN_CPP_VER  _MSVC_LANG
	#define _KSN_FORCEINLINE __forceinline

	#ifndef _KSN_IS_64
		#ifdef _WIN64
			#define _KSN_IS_64 1
		#else
			#define _KSN_IS_64 0
		#endif
	#endif

	#ifndef LDBL_DECIMAL_DIG
		#define LDBL_DECIMAL_DIG DBL_DECIMAL_DIG
	#endif

#elif defined _KSN_COMPILER_GCC

#ifndef _KSN_IS_64
	#if defined __LP64__ || defined __LLP64__
		#define _KSN_IS_64 1
	#else
		#define _KSN_IS_64 0
	#endif
#endif

	#define _KSN_FORCEINLINE __always_inline

#else

#ifndef _KSN_IS_64
	#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
			#define _KSN_IS_64 1
		#else
			#define _KSN_IS_64 0
	#endif
#endif

	#define _KSN_FORCEINLINE inline

#endif

#undef _KSN_FORCEINLINE



#if _KSN_IS_64
static_assert(sizeof(void*) == 8, "Failed to guess whether compiling for 64 bits or not. #define _KSN_IS_64 to be 1 or 0 according to the enviroment your are compiling for");
static_assert(SIZE_MAX == UINT64_MAX, "Failed to guess whether compiling for 64 bits or not. #define _KSN_IS_64 to be 1 or 0 according to the enviroment your are compiling for");
#else
static_assert(sizeof(void*) == 4, "Failed to guess whether compiling for 64 bits or not. #define _KSN_IS_64 to be 1 or 0 according to the enviroment your are compiling for");
static_assert(SIZE_MAX == UINT32_MAX, "Failed to guess whether compiling for 64 bits or not. #define _KSN_IS_64 to be 1 or 0 according to the enviroment your are compiling for");
#endif



#ifndef _KSN_NO_BYTE_SIZE_CHECK
static_assert(CHAR_BIT == 8, "The size of a byte does not equal to 8 bits. You can suppress this warning by #define _KSN_NO_BYTE_SIZE_CHECK before any ksn header but you should not expect this library to work properly");
#endif



#ifndef _KSN_COMPILER_MSVC

	#define _KSN_CPP_VER __cplusplus

	#ifndef _STD
		#define _STD ::std::
	#endif

	#ifndef _STD_BEGIN
		#define _STD_BEGIN namespace std {
	#endif

	#ifndef _STD_END
		#define _STD_END }
	#endif

#endif



#if _KSN_CPP_VER < 201103L
	#ifndef nullptr
		#define nullptr NULL
	#endif
#endif


#if __cpp_attributes != 0 && defined __has_cpp_attribute
	#if __has_cpp_attribute(nodiscard)

		#define _KSN_NODISCARD [[nodiscard]]

	#elif defined _KSN_COMPILER_GNU_

		#if __has_cpp_attribute(gnu::warn_unused_result)
		#define _KSN_NODISCARD [[gnu::warn_unused_result]]
		#endif

	#endif
#endif

#ifndef _KSN_NODISCARD
	#define _KSN_NODISCARD
#endif

#ifdef _KSN_NODISCARD
#define _KSN_NODISCARD_BECAUSE(reason) [[nodiscard(#reason)]]
#else
#define _KSN_NODISCARD_BECAUSE(reason)
#endif



#ifdef __cpp_constexpr
	#define _KSN_CONSTEXPR constexpr
#else
	#define _KSN_CONSTEXPR
#endif

#ifdef __cpp_if_constexpr
	#define _KSN_CONSTEXPR_CONDITION constexpr
#else
	#define _KSN_CONSTEXPR_CONDITION
#endif



#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
	#define _KSN_HAS_EXCEPTIONS 1
	#define _KSN_TRY try {
	#define _KSN_CATCH_UNNAMED(type) } catch(type) {
	#define _KSN_CATCH(type, name) } catch(type name) { ((void)(name));
	#define _KSN_CATCH_ALL } catch(...) {
	#define _KSN_CATCH_END }
	#define _KSN_RAISE(exception) { throw (exception); abort(); }((void)0)
	#define _KSN_RERAISE { throw; } ((void)0)
#else
	#define _KSN_HAS_EXCEPTIONS 0
	#define _KSN_TRY if _KSN_CONSTEXPR_CONDITION (true) {
	#define _KSN_CATCH_UNNAMED(type) } else if _KSN_CONSTEXPR_CONDITION (false) {
	#define _KSN_CATCH(type, name) } else if _KSN_CONSTEXPR_CONDITION (false) { type name = *(std::add_pointer_t<std::remove_reference_t<type>>)nullptr;
	#define _KSN_CATCH_ALL } else if _KSN_CONSTEXPR_CONDITION (false) {
	#define _KSN_CATCH_END }
	#define _KSN_RAISE(exception) { void(exception); abort(); }((void)0)
	#define _KSN_RERAISE { abort(); } ((void)0)
#endif





#if __has_cpp_attribute(likely)
#define _KSN_LIKELY [[likely]]
#else
#define _KSN_LIKELY
#endif

#if __has_cpp_attribute(unlikely)
#define _KSN_UNLIKELY [[unlikely]]
#else
#define _KSN_UNLIKELY
#endif





#ifndef _KSN_NO_TRIVIAL_TYPES_SIZES_CHECK_

	#ifndef _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_
		static_assert(sizeof(int8_t) == 1, "The size of a 1-byte variable does not equal to 1 byte. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_");
		static_assert(sizeof(int16_t) == 2, "The size of a 2-byte variable does not equal to 2 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_");
		static_assert(sizeof(int32_t) == 4, "The size of a 4-byte variable does not equal to 4 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_"); //-V112
		static_assert(sizeof(int64_t) == 8, "The size of an 8-byte variable does not equal to 2 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_");

		static_assert(sizeof(uint8_t) == 1, "The size of a 1-byte variable does not equal to 1 byte. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_");
		static_assert(sizeof(uint16_t) == 2, "The size of a 2-byte variable does not equal to 2 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_");
		static_assert(sizeof(uint32_t) == 4, "The size of a 4-byte variable does not equal to 4 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_"); //-V112
		static_assert(sizeof(uint64_t) == 8, "The size of an 8-byte variable does not equal to 8 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_");
	#endif

	#ifndef _KSN_NO_TRIVIAL_FP_TYPES_SIZES_CHECK_
		static_assert(sizeof(float) == 4, "The size of a float type variable does not equal to 4 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_FP_TYPES_SIZES_CHECK_"); //-V112
		static_assert(sizeof(double) == 8, "The size of a double type variable does not equal to 8 bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_FP_TYPES_SIZES_CHECK_");
	#endif

#endif



_KSN_BEGIN

_KSN_DETAIL_BEGIN
_KSN_DETAIL_END

//A type instance of which when passed specifies that the created object shouldn't be initialized
struct uninitialized_t {};

#if defined __cpp_char8_t && defined __cpp_lib_char8_t
using char8 = char8_t;
#else
using char8 = char;
#endif

_KSN_END





#else //!__cplusplus

#error <ksn/ksn.hpp> can only be included into files compiled with a C++ compiler

#endif //!__cplusplus



#endif //!_KSN_HPP_
