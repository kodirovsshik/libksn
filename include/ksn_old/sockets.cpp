//All this included stuff is only to make VS not to swear about undefined identifiers

#include <ksn/ksn.hpp>
#include <ksn/sockets.hpp>

#include <iostream>

///////////////////////////////////////////////
//
//		Template stuff implementation
//
///////////////////////////////////////////////

_KSN_BEGIN


template<typename T>
T packet::get()
{
	T res;
	*this >> res;
	return res;
}


template<class T>
class socket_alloc_t
{
public:
	template<class U>
	struct rebind
	{
		typedef socket_alloc_t<U> other;
	};

	using value_type = T;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using is_always_equeal = std::true_type;

	T* address(const T& data)
	{
		return std::addressof(data);
	}
	T* allocate(size_t count)
	{
		T* ret = (T*)socket_allocator(count * sizeof(T));
		return ret;
	}
	T* allocate(size_t count, const void*)
	{
		return this->allocate(count);
	}
	void deallocate(T* addr, size_t count)
	{
		socket_deallocator(addr, count * sizeof(T));
	}
	template<class ...Types>
	void construct(T* addr, Types ...args)
	{
		new ((void*)addr) T(args...);
	}
	void destruct(T* addr)
	{
		addr->~T();
	}
};

template<typename CharT, class Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& stream, ip_address addr)
{
	std::basic_string<CharT, Traits, socket_alloc_t<CharT>> temp;
	stream >> temp;
	addr = ip_address(temp);
	return stream;
}


_KSN_END