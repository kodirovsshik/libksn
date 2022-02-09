
#ifndef _KSN_STUFF_HPP_
#define _KSN_STUFF_HPP_


#include <ksn/ksn.hpp>

#include <chrono>
#include <type_traits>
#include <utility>

#include <stdio.h>



_KSN_BEGIN



template<class T, size_t N>
constexpr size_t countof(const T(&)[N]) noexcept;





template<class ...args_t>
constexpr void nop(args_t&&...) noexcept;





size_t c16len(const char16_t* String);

size_t c32len(const char32_t* String);





template<class RetType, class ArgType>
constexpr RetType&& convert_or_create(ArgType&&);

template<class RetType, class ArgType>
constexpr RetType&& convert_or_forward(ArgType&&, const RetType&);





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

	malloc_guard& operator=(const malloc_guard&) = delete;
	malloc_guard& operator=(malloc_guard&&) noexcept;

	bool reserve(size_t n_adresses) noexcept;
	bool reserve_more(size_t n_new_adresses) noexcept;

	void* alloc(size_t memsize) noexcept;
	void free(void*) noexcept;
};





void dynamic_assert(int nonzero, const char* expr, const char* msg, int line, const char* file, ...);
#if _KSN_IS_DEBUG_BUILD
#define ksn_dynamic_assert(true_expr, msg, ...) ksn::dynamic_assert(!!(true_expr), #true_expr, msg, __LINE__, __FILE__, __VA_ARGS__);
#else
#define ksn_dynamic_assert(...)
#endif



void memset_parallel(void* void_dst, uint8_t byte, size_t size);





template<class int_t>
constexpr int_t align_up(const int_t& value, const int_t& alignment) noexcept;
template<class int_t>
constexpr int_t align_down(const int_t& value, const int_t& alignment) noexcept;

template<class int_t>
constexpr int_t align_up_log2(const int_t& value, size_t log2_alignment) noexcept;
template<class int_t>
constexpr int_t align_down_log2(const int_t& value, size_t log2_alignment) noexcept;

template<class int_t>
constexpr int_t alignment_up_miss(const int_t& value, const int_t& alignment) noexcept;
template<class int_t>
constexpr int_t alignment_down_miss(const int_t& value, const int_t& alignment) noexcept;

template<class int_t>
constexpr int_t alignment_up_log2_miss(const int_t& value, const int_t& log2_alignment) noexcept;
template<class int_t>
constexpr int_t alignment_down_log2_miss(const int_t& value, const int_t& log2_alignment) noexcept;

template <class int_t>
constexpr bool alignment_check(const int_t& value, const int_t& alignment) noexcept;
template <class int_t>
constexpr bool alignment_check_log2(const int_t& value, const int_t& log2_alignment) noexcept;





template<class trivial_t>
bool file_read_bin_data(FILE* fd, trivial_t& data) noexcept;

template<class trivial_t>
constexpr void bswap(trivial_t& data) noexcept;





inline volatile int raise_DE();





_KSN_END





//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//Template, inline and constexpr stuff implementation

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////





_KSN_BEGIN



_KSN_DETAIL_BEGIN

template<class sizeable_t>
concept sizeable = requires(sizeable_t sizeable) { sizeable.size(); };

_KSN_DETAIL_END

template<class T, size_t N>
constexpr size_t countof(const T(&)[N]) noexcept
{
	return N;
}

template<detail::sizeable container_t>
constexpr auto countof(container_t&& container) noexcept
{
	return container.size();
}





template<class ...args_t>
constexpr void nop(args_t&&...) noexcept {}





inline volatile int raise_DE()
{
	int x = 0;
	return x / x;
}





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
constexpr RetType&& convert_or_create(ArgType&& to_be_converted)
{
	return detail::convert_or_create_helper<std::remove_reference_t<RetType>>
		(std::is_constructible<RetType, decltype(to_be_converted)>(), to_be_converted);
}

template<class RetType, class ArgType>
constexpr RetType&& convert_or_forward(ArgType&& to_be_converted, const RetType& to_be_returned_on_convertion_failure)
{
	return detail::convert_or_forward_helper<std::remove_reference_t<RetType>>
		(std::is_constructible<RetType, decltype(to_be_converted)>(), to_be_converted, to_be_returned_on_convertion_failure);
}





template<class int_t>
constexpr int_t align_up(const int_t& value, const int_t& alignment) noexcept
{
	return value + alignment_up_miss(value, alignment);
}
template<class int_t>
constexpr int_t align_down(const int_t& value, const int_t& alignment) noexcept
{
	return value - (value % alignment);
}


template<class int_t>
constexpr int_t align_up_log2(const int_t& value, size_t log2_alignment) noexcept
{
	//TODO: fix
	return value + alignment_up_log2_miss(value, log2_alignment);
}
template<class int_t>
constexpr int_t align_down_log2(const int_t& value, size_t log2_alignment) noexcept
{
	return value - (value >> log2_alignment);
}


template<class int_t>
constexpr int_t alignment_up_miss(const int_t& value, const int_t& alignment) noexcept
{
	int_t remainder = value % alignment;
	return remainder == 0 ? 0 : alignment - remainder;
}
template<class int_t>
constexpr int_t alignment_down_miss(const int_t& value, const int_t& alignment) noexcept
{
	return value % alignment;
}


template<class int_t>
constexpr int_t alignment_up_log2_miss(const int_t& value, const int_t& log2_alignment) noexcept
{
	int_t remainder = value & ((int_t(1) << log2_alignment) - 1);
	return remainder == 0 ? 0 : (int_t(1) << log2_alignment) - remainder;
}
template<class int_t>
constexpr int_t alignment_down_log2_miss(const int_t& value, const int_t& log2_alignment) noexcept
{
	return value & ((int_t(1) << log2_alignment) - 1);
}





template <class int_t>
constexpr bool alignment_check(const int_t& value, const int_t& alignment) noexcept
{
	return (value % alignment) == 0;
}
template <class int_t>
constexpr bool alignment_check_log2(const int_t& value, const int_t& log2_alignment) noexcept
{
	int_t magic_const = (int_t(1) << log2_alignment) - 1;
	return (value & magic_const) == 0;
}





template<class trivial_t>
bool file_read_bin_data(FILE* fd, trivial_t& data) noexcept
{
	if (!fd) return false;

	static constexpr size_t size = sizeof(data);
	uint8_t* p = (uint8_t*)std::addressof(data);

	if (fread(p, 1, size, fd) != size)
		return false;

	return true;
}

#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 6294)
#endif

template<class trivial_t>
constexpr void bswap(trivial_t& data) noexcept
{
	static constexpr size_t size = sizeof(data);
	uint8_t* p = (uint8_t*)std::addressof(data);

	//Screw you, big endian
	for (size_t i = 0; i < size / 2; ++i)
	{
		std::iter_swap(p + i, p + size - i - 1);
	}
}

#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif

_KSN_END



#endif //!_KSN_STUFF_HPP_
