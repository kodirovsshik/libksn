
#ifndef _KSN_FAST_PIMPL_HPP_
#define _KSN_FAST_PIMPL_HPP_



#include <ksn/ksn.hpp>

#include <type_traits>
#include <utility>



_KSN_BEGIN

template<class T, size_t size, size_t alignment>
class fast_pimpl
{

private:
		
	using my_t = fast_pimpl<T, size, alignment>;
	


private:

	std::aligned_storage_t<size, alignment> m_object;



private:
	
	template<size_t actual_size, size_t actual_alignment>
	constexpr static void validate(int) noexcept
	{
		static_assert(actual_size == size);
		static_assert(actual_alignment == alignment);
	}



public:
	
	T* ptr() noexcept
	{
		return reinterpret_cast<T*>(&this->m_object);
	}
	const T* ptr() const noexcept
	{
		return reinterpret_cast<const T*>(&this->m_object);
	}

	T* operator->() noexcept
	{
		return this->ptr();
	}
	const T* operator->() const noexcept
	{
		return this->ptr();
	}
	
	T& operator*() noexcept
	{
		return *this->ptr();
	}
	const T& operator*() const noexcept
	{
		return *this->ptr();
	}



	template<class... args_t>
	fast_pimpl(args_t&& ...args)
	{
		new ((void*)this->ptr()) T(std::forward<args_t>(args)...);
	}
	fast_pimpl(const my_t& other)
	{
		new (this->ptr()) T{ *other };
	}
	fast_pimpl(my_t&& other) noexcept(false)
	{
		new (this->ptr()) T{ std::move(*other) };
	}

	
	~fast_pimpl()
	{
		my_t::validate<sizeof(T), alignof(T)>(0);
		this->ptr()->~T();
	}

	fast_pimpl& operator=(const my_t& other)
	{
		*this->ptr() = *other.ptr();
		return *this;
	}

	fast_pimpl& operator=(my_t&& other)
	{
		std::iter_swap(this->ptr(), other.ptr());
		return *this;
	}
};

_KSN_END


#endif //!_KSN_FAST_PIMPL_HPP_
