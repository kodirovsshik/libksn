
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
#define _KSN_GRAPHICS_BEGIN namespace graphics{
#define _KSN_GRAPHICS_END }
#define _KSN_DETAIL_BEGIN namespace detail{
#define _KSN_DETAIL_END }
#define _KSN_CL_BEGIN namespace cl{
#define _KSN_CL_END }





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

#include <vcruntime_new.h>

	#define _KSN_CPP_VER _MSVC_LANG

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

	#define _NOMINMAX
	#undef min
	#undef max

#elif defined _KSN_COMPILER_GCC

#ifndef _KSN_IS_64
	#if defined __LP64__ || defined __LLP64__
		#define _KSN_IS_64 1
	#else
		#define _KSN_IS_64 0
	#endif
#endif

#else

#ifndef _KSN_IS_64
	#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
			#define _KSN_IS_64 1
		#else
			#define _KSN_IS_64 0
	#endif
#endif

#endif


#if defined _DEBUG || defined DEBUG
	#define __KSN_DEBUG
#endif

#if defined NDEBUG || defined _NDEBUG
	#define __KSN_RELEASE
#endif

#if defined __KSN_DEBUG && !defined __KSN_RELEASE
	#define _KSN_IS_DEBUG_BUILD 1
#elif defined __KSN_RELEASE && !defined __KSN_DEBUG
	#define _KSN_IS_DEBUG_BUILD 0
#elif !defined __KSN_DEBUG && !defined __KSN_RELEASE
	#ifndef _KSN_NO_IS_DEBUG_WARNING
		#error Failed to define _KSN_IS_DEBUG_BUILD. You can #define _KSN_NO_IS_DEBUG_WARNING to acknowledge the compiler that you've recived this warning (undefined macro will be defined as 0)
	#endif
	#define _KSN_IS_DEBUG_BUILD 0
#endif



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





#ifndef _KSN_IS_DEBUG_BUILD
#error Failed to define _KSN_IS_DEBUG_BUILD. Please #define _DEBUG or NDEBUG
#endif

#ifndef _KSN_IS_64
	#error Failed to define _KSN_IS_64. Please predefine it as 1 if compiling for 64 bits and 0 for 32 bits (Note: <32 or >64 platforms are not supported)
#endif



//Expression that will only be executed on debug builds
#define _KSN_DEBUG_EXPR(expression) if constexpr (_KSN_IS_DEBUG_BUILD) { expression; }; ((void)0)




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



//A type instance of which when passed specifies that the created object shouldn't be initialized (if class supports it)
struct uninitialized_t {} static constexpr uninitialized;



_KSN_END





#else //!__cplusplus

#error <ksn/ksn.hpp> can only be included into files compiled with a C++ compiler

#endif //!__cplusplus



#endif //!_KSN_HPP_
