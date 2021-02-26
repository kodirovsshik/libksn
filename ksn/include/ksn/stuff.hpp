
#pragma once

#ifndef _FUNCTION_HPP_
#define _FUNCTION_HPP_


#include <ksn/ksn.hpp>
#include <locale>
#include <clocale>
#include <algorithm>
#include <mutex>
#include <vector>
#include <chrono>





_KSN_BEGIN



template<class T, size_t N>
constexpr size_t countof(const T(&)[N]) noexcept
{
	return N;
}





template<class ...args_t>
constexpr void nop(args_t&&...) noexcept
{

}





size_t c16len(const char16_t* String);

size_t c32len(const char32_t* String);





template<class RetType, class ArgType>
RetType&& convert_or_create(ArgType&&);

template<class RetType, class ArgType>
RetType&& convert_or_forward(ArgType&&, const RetType&);





class _memory_dump_t
{
private:
	_memory_dump_t() {}
	_memory_dump_t(const _memory_dump_t&) = delete;
	_memory_dump_t(_memory_dump_t&&) = delete;

public:
	using flag_t = uint_fast8_t;

	static _memory_dump_t& instance()
	{
		static _memory_dump_t obj;
		return obj;
	}


	static constexpr flag_t lower_case = 1 << 0;
	static constexpr flag_t no_space = 1 << 1;

	
	size_t operator()(const void* memory, size_t bytes, size_t bytes_per_line = -1, flag_t flags = 0, FILE* f = stdout);

} static &memory_dump = _memory_dump_t::instance();





struct malloc_guard
{
	uintptr_t* m_data;
	size_t m_count, m_capacity;

	~malloc_guard() noexcept;
	malloc_guard() noexcept;
	malloc_guard(const malloc_guard&) = delete;
	malloc_guard(malloc_guard&&) noexcept;

	bool reserve(size_t n_adresses) noexcept;
	bool reserve_more(size_t n_new_adresses) noexcept;

	void* alloc(size_t memsize) noexcept;
	void free(void*) noexcept;
};





//Tests if there is any value in memory other than the one passed into the function
const void* memnotchr(const void* block, uint8_t value, size_t length);





void dynamic_assert(int nonzero, const char* expr, const char* msg, int line, const char* file, ...);
#define ksn_dynamic_assert(true_expr, msg, ...) ksn::dynamic_assert(!!(true_expr), #true_expr, msg, __LINE__, __FILE__, __VA_ARGS__);





_KSN_END





//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//Template stuff implementation

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////





_KSN_BEGIN



template<class ReturnType, class callable, class... Arguments>
uint64_t measure_running_time(ReturnType& p, callable&& functor, Arguments&&... args)
{
	auto* f = _STD chrono::high_resolution_clock::now;

	std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;

	t1 = f();
	p = functor(std::forward<Arguments&&>(args)...);
	t2 = f();

	return (_STD chrono::duration_cast<_STD chrono::nanoseconds>(t2 - t1)).count();
}

template<class Callable, class... Arguments>
uint64_t measure_running_time_no_return(Callable&& functor, Arguments&&... args)
{
	auto* f = _STD chrono::high_resolution_clock::now;

	std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;

	t1 = f();
	functor(std::forward<Arguments&&>(args)...);
	t2 = f();

	return (_STD chrono::duration_cast<_STD chrono::nanoseconds>(t2 - t1)).count();
}



_KSN_DETAIL_BEGIN



template<class RetType, class ArgType>
RetType&& convert_or_create_helper(std::false_type, ArgType&&)
{
	return std::forward<RetType>(RetType());
}

template<class RetType, class ArgType>
RetType&& convert_or_create_helper(std::true_type, ArgType&& arg)
{
	return std::forward<RetType>(RetType(arg));
}



template<class RetType, class ArgType>
RetType&& convert_or_forward_helper(std::false_type, ArgType&&, const RetType& default_val)
{
	return std::forward<RetType>(default_val);
}

template<class RetType, class ArgType>
RetType&& convert_or_forward_helper(std::true_type, ArgType&& arg, const RetType&)
{
	return std::forward<RetType>(arg);
}



_KSN_DETAIL_END



template<class RetType, class ArgType>
RetType&& convert_or_create(ArgType&& to_be_converted)
{
	return detail::convert_or_create_helper<std::remove_reference_t<RetType>>
		(std::is_constructible<RetType, decltype(to_be_converted)>(), to_be_converted);
}

template<class RetType, class ArgType>
RetType&& convert_or_forward(ArgType&& to_be_converted, const RetType& to_be_returned_on_convertion_failure)
{
	return detail::convert_or_forward_helper<std::remove_reference_t<RetType>>
		(std::is_constructible<RetType, decltype(to_be_converted)>(), to_be_converted, to_be_returned_on_convertion_failure);
}



_KSN_END

#endif //_FUNCTION_HPP_
