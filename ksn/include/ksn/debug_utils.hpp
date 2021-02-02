
#pragma once

#ifndef _KSN_DEBUG_UTILS_HPP_
#define _KSN_DEBUG_UTILS_HPP_


#include <stdio.h>

#include <list>
#include <stdexcept>

#include <ksn/ksn.hpp>





_KSN_BEGIN



struct copy_debugger
{
public:
	volatile uint32_t signature;

	static FILE* log_file; //stderr by default
	static bool force_debug_breaks; //false by default
	static bool break_at_check_error; //false by default

	copy_debugger();
	copy_debugger(const copy_debugger&);
	copy_debugger(copy_debugger&&);
	~copy_debugger();

	copy_debugger& operator=(const copy_debugger&);
	copy_debugger& operator=(copy_debugger&&);

	static void _break();

	void log() const;

	void check() const;
};





struct collecting_allocator_tracker
{
	static void track_start();
	static ptrdiff_t track_stop();

private:
	static std::list<ptrdiff_t> track_layers;

	collecting_allocator_tracker() = delete;
	virtual ~collecting_allocator_tracker() = 0 {};

	template<class T, class Alloc>
	friend class collecting_allocator;
};

template<class T, class Alloc = std::allocator<T>>
class collecting_allocator
{
	Alloc alloc;

public:
	collecting_allocator() noexcept {};
	collecting_allocator(const collecting_allocator&) noexcept {};
	collecting_allocator(collecting_allocator&&) noexcept {};

	collecting_allocator& operator=(const collecting_allocator&) { return *this; }
	collecting_allocator& operator=(collecting_allocator&&) { return *this; }

	T* allocate(size_t s)
	{
		T* p = std::allocator_traits<Alloc>::allocate(alloc, s);

		if (p && collecting_allocator_tracker::track_layers.size())
		{
			collecting_allocator_tracker::track_layers.back() += s * sizeof(T);
		}

		return p;
	}

	void deallocate(T* p, size_t s)
	{
		std::allocator<T> alloc;

		if (p && s && collecting_allocator_tracker::track_layers.size())
		{
			collecting_allocator_tracker::track_layers.back() -= s * sizeof(T);
		}

		std::allocator_traits<decltype(alloc)>::deallocate(alloc, p, s);
	}

	bool operator==(const collecting_allocator& other) const noexcept
	{
		return true;
	}
	bool operator!=(const collecting_allocator& other) const noexcept
	{
		return false;
	}

	using value_type = T;
	using is_always_equal = std::true_type;
};


_KSN_END





#endif //!_KSN_DEBUG_UTILS_HPP_
