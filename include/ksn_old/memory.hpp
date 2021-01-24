#pragma once

#ifndef _KSN_MEMORY_HPP_
#define _KSN_MEMORY_HPP_



#include <ksn/ksn.hpp>



_KSN_BEGIN

/*
Parameter _wrapper_number is used to tell apart different allocator instances of the same type
Use different parameter values for different instances
*/
template<class _allocator_type, size_t _wrapper_number>
class allocator_instance_wrapper
{
	static _allocator_type* p_allocator;

public:
	typedef void* (alloc_func)(size_t);
	typedef void (dealloc_func)(void*, size_t);

	using my_type = allocator_instance_wrapper<_allocator_type, _wrapper_number>;



private:

	static void* alloc(size_t x)
	{
		using traits = std::allocator_traits<allocator_type>;

		typename traits::pointer p = traits::allocate(*p_allocator, x / sizeof(typename traits::value_type));

		return static_cast<void*>(p);
	}
	static void dealloc(void* p, size_t x)
	{
		using traits = std::allocator_traits<allocator_type>;

		return traits::deallocate(*p_allocator, static_cast<typename traits::pointer>(p), x / sizeof(traits::value_type));
	}



public:

	using next_wrapper = allocator_instance_wrapper <_allocator_type, _wrapper_number + 1>;
	using previous_wrapper = allocator_instance_wrapper <_allocator_type, _wrapper_number - 1>;

	using allocator_type = _allocator_type;
	static constexpr size_t wrapper_number = _wrapper_number;



	static alloc_func* wrap_allocate(allocator_type& alloc)
	{
		my_type::p_allocator = &alloc;
		return static_cast<alloc_func*>(&my_type::alloc);
	}
	
	static dealloc_func* wrap_deallocate(allocator_type& alloc)
	{
		my_type::p_allocator = &alloc;
		return static_cast<dealloc_func*>(&my_type::dealloc);
	}
};

template<class _allocator_type, size_t _wrapper_number>
_allocator_type* allocator_instance_wrapper< _allocator_type, _wrapper_number>::p_allocator = nullptr;





template<class _allocator_type>
class allocator_wrapper
{
public:
	typedef void* (alloc_func)(size_t);
	typedef void (dealloc_func)(void*, size_t);

	using my_type = allocator_wrapper<_allocator_type>;


private:

	static void* allocate(size_t n)
	{
		_allocator_type allocator;
		using traits = std::allocator_traits< _allocator_type>;

		typename traits::pointer p = traits::allocate(allocator, n / sizeof(typename traits::value_type));

		return static_cast<void*>(p);
	}

	static void deallocate(void* p, size_t n)
	{
		_allocator_type allocator;
		using traits = std::allocator_traits< _allocator_type>;

		traits::deallocate(allocator, static_cast<typename traits::pointer>(p), n / sizeof(typename traits::value_type));
	}



public:

	static alloc_func* wrap_allocate()
	{
		return static_cast<alloc_func*>(&my_type::allocate);
	}
	static dealloc_func* wrap_deallocate()
	{
		return static_cast<dealloc_func*>(&my_type::deallocate);
	}
};






template<class T>
T** allocate_2d_array(size_t h, size_t w, void* (*p_allocator_func)(size_t) = &::operator new)
{
	T** memory = (T**)p_allocator_func(sizeof(void*) * h + sizeof(T) * h * w);
	T* p_array = (T*)(memory + h);
	for (T** p = memory; p != memory + h; ++p)
	{
		*p = p_array;
		p_array += w;
	}
	return memory;
}


_KSN_END


#endif //_KSN_MEMORY_HPP_