
#ifndef _KSN_FAST_PIMPL_HPP_
#define _KSN_FAST_PIMPL_HPP_



#include <ksn/ksn.hpp>

#include <type_traits>

template<class T>
struct identity_inherit
{
	template<class ...>
	struct type : public T
	{
	};
};

//template<class T>
//template<class ... params_t>
//using indentity_t = template identity<T>::helper<params_t>;


_KSN_BEGIN

template<class T, size_t size, size_t alignment,
	bool is_nothrow_copy_constructible = false,
	bool is_nothrow_move_constructible = false,
	bool is_nothrow_destructible = false,
	bool is_nothrow_constructible = false
>
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
	fast_pimpl(args_t&& ...args) noexcept(is_nothrow_constructible)
	{
		new (this->ptr()) T(std::forward<args_t>(args)...);
	}
	fast_pimpl(const fast_pimpl& other) noexcept(is_nothrow_copy_constructible)
	{
		new (this->ptr()) T(*other);
	}
	fast_pimpl(fast_pimpl&& other) noexcept(is_nothrow_move_constructible)
	{
		new (this->ptr()) T(std::move(*other));
	}
	
	~fast_pimpl() noexcept(is_nothrow_destructible)
	{
		my_t::validate<sizeof(T), alignof(T)>(0);
		this->ptr()->~T();
	}
};

_KSN_END


#endif //!_KSN_FAST_PIMPL_HPP_
