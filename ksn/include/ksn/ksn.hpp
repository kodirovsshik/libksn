
#ifndef _KSN_HPP_
#define _KSN_HPP_

#ifdef __cplusplus





#include <stdint.h>
#include <float.h>
#include <limits.h>





//Redefine some basic ksn defines

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




//Well, this is not an "external linkage" function in fact, but it behaves
//as an external linkage variable and can be defined the same way
//in many files without causing linker errors
#define KSN_EXTERNAL_LINKAGE_FUNCTION inline





//Check for current compiler

#if defined __GNUC__ || defined __GNUG__
#define _KSN_COMPILER_GNU
#endif


#if defined _MSVC_LANG

	#define _KSN_COMPILER_MSVC

#elif defined __clang__

	#define _KSN_COMPILER_CLANG

#elif defined _KSN_COMPILER_GNU

	#define _KSN_COMPILER_GCC

#else

	#define _KSN_COMPILER_UNKNOWN

#endif





//Do compiler-dependent preprocessor stuff
//Stage 1: do things depending on what is the current compiler

#ifdef _KSN_COMPILER_MSVC


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

	#define _KSN_RESTRICT __restrict


#elif defined _KSN_COMPILER_GNU


	#ifndef _KSN_IS_64
		#if defined __LP64__ || defined __LLP64__
			#define _KSN_IS_64 1
		#else
			#define _KSN_IS_64 0
		#endif
	#endif

	#define _KSN_RESTRICT __restrict


#else


	#ifndef _KSN_IS_64
		#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
			#define _KSN_IS_64 1
		#else
			#define _KSN_IS_64 0
		#endif
	#endif


#endif





//Do compiler-dependent preprocessor stuff
//Stage 2: do things depending on what is NOT the current compiler

#ifndef _KSN_COMPILER_MSVC

	#define _KSN_CPP_VER __cplusplus

#endif





//Check if the current build is a debug or a release build

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
		#error Failed to define _KSN_IS_DEBUG_BUILD because neither debug nor release macro is defined. \
Define _KSN_IS_DEBUG_BUILD yourself to be 0 or 1 before any ksn header or reconfigure your project settings. \
You can #define _KSN_NO_IS_DEBUG_WARNING to acknowledge the compiler that you've recived this warning (release build will be assumed)
	#endif

	#define _KSN_IS_DEBUG_BUILD 0

#else //defined __KSN_DEBUG && defined __KSN_RELEASE

	#ifndef _KSN_IS_DEBUG_BUILD
		#error Failed to define _KSN_IS_DEBUG_BUILD because both debug and release macros are defined. \
Define _KSN_IS_DEBUG_BUILD yourself to be 0 or 1 before any ksn header or reconfigure your project settings
	#endif

#endif


//Temporary macros
#undef __KSN_DEBUG
#undef __KSN_RELEASE





//Feature testing

#if __cpp_attributes != 0 && defined __has_cpp_attribute

	#if __has_cpp_attribute(nodiscard) >= 201603L
		#define _KSN_NODISCARD [[nodiscard]]
	#endif

	#if __has_cpp_attribute(nodiscard) >= 201907L
		#define _KSN_NODISCARD_BECAUSE(reason) [[nodiscard(#reason)]]
	#endif

	#if __has_cpp_attribute(likely) >= 201803L
		#define _KSN_LIKELY [[likely]]
	#endif

	#if __has_cpp_attribute(unlikely) >= 201803L
		#define _KSN_UNLIKELY [[unlikely]]
	#endif

#endif





//Expression that will only be executed on debug builds
#define _KSN_DEBUG_EXPR(expression) if constexpr (_KSN_IS_DEBUG_BUILD) { expression; } else []{}()





//Conditional exceptions handling

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





//Check all the necessary assumtrions

#ifndef _KSN_NO_TRIVIAL_TYPES_SIZES_CHECK_

	#define _KSN_TRIVIAL_INT_SIZE_FAIL(bytes, s) \
"The size of a " #bytes "-byte variable does not equal to " #bytes " byte" #s ". The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_"

	#ifndef _KSN_NO_TRIVIAL_INT_TYPES_SIZES_CHECK_
		static_assert(sizeof(int8_t) == 1 && sizeof(uint8_t) == 1, _KSN_TRIVIAL_INT_SIZE_FAIL(1, ));
		static_assert(sizeof(int16_t) == 2 && sizeof(uint16_t) == 2, _KSN_TRIVIAL_INT_SIZE_FAIL(2, s));
		static_assert(sizeof(int32_t) == 4 && sizeof(uint32_t) == 4, _KSN_TRIVIAL_INT_SIZE_FAIL(4, s));
		static_assert(sizeof(int64_t) == 8 && sizeof(uint64_t) == 8, _KSN_TRIVIAL_INT_SIZE_FAIL(8, s));
	#endif

	#define _KSN_TRIVIAL_FLOAT_SIZE_FAIL(type, bytes) \
"The size of \"" #type "\" type variable does not equal to " #bytes " bytes. The library is not guaranteed to work properly. To suppress this error, define _KSN_NO_TRIVIAL_FP_TYPES_SIZES_CHECK_"

	#ifndef _KSN_NO_TRIVIAL_FP_TYPES_SIZES_CHECK_
		static_assert(sizeof(float) == 4, _KSN_TRIVIAL_FLOAT_SIZE_FAIL(float, 4));
		static_assert(sizeof(double) == 8, _KSN_TRIVIAL_FLOAT_SIZE_FAIL(double, 8));
	#endif

	#undef _KSN_TRIVIAL_INT_SIZE_FAIL
	#undef _KSN_TRIVIAL_FLOAT_SIZE_FAIL

#endif





//Check all the necessary definitions

#ifndef _KSN_IS_64

	#error Failed to define _KSN_IS_64. Please predefine it as 1 if compiling for 64 bits and 0 for 32 bits (Note: <32 or >64 platforms are not supported)

#else

	#define _KSN_IS_64_FAILURE_MESSAGE "Failed to guess whether compiling for 64 bits or not. #define _KSN_IS_64 to be 1 or 0 according to the enviroment your are compiling for"

	#if _KSN_IS_64
		static_assert(sizeof(void*) == 8, _KSN_IS_64_FAILURE_MESSAGE);
		static_assert(SIZE_MAX == UINT64_MAX, _KSN_IS_64_FAILURE_MESSAGE);
	#else
		static_assert(sizeof(void*) == 4, _KSN_IS_64_FAILURE_MESSAGE);
		static_assert(SIZE_MAX == UINT32_MAX, _KSN_IS_64_FAILURE_MESSAGE);
	#endif

	#undef _KSN_IS_64_FAILURE_MESSAGE

#endif


#ifndef _KSN_NO_BYTE_SIZE_CHECK
	static_assert(CHAR_BIT == 8, "The size of a byte does not equal to 8 bits. You can suppress this warning by #define _KSN_NO_BYTE_SIZE_CHECK before any ksn header but you should not expect this library to work properly");
#endif





#ifndef _KSN_NODISCARD
	#define _KSN_NODISCARD
#endif

#ifndef _KSN_NODISCARD_BECAUSE
	#define _KSN_NODISCARD_BECAUSE(reason)
#endif

#ifndef _KSN_LIKELY
	#define _KSN_LIKELY
#endif

#ifndef _KSN_UNLIKELY
	#define _KSN_UNLIKELY
#endif

#ifndef _KSN_RESTRICT
	#define _KSN_RESTRICT
#endif





//Minimal definition of ksn namespace

_KSN_BEGIN

_KSN_DETAIL_BEGIN
_KSN_DETAIL_END

_KSN_END





#else //if !__cplusplus

#error <ksn/ksn.hpp> can only be included into files compiled with a C++ compiler

#endif //!__cplusplus



#endif //!_KSN_HPP_
