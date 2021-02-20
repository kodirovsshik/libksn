
#ifndef _KSN_PPVECTOR_HPP_
#define _KSN_PPVECTOR_HPP_


#include <ksn/ksn.hpp>
#include <stdlib.h>
#include <type_traits>


_KSN_BEGIN

//Poor programmer's vector
//std::vector class for trivial types (only!!)

template<typename T>
struct ppvector
{
#ifndef _KSN_PPVECTOR_NO_TRIVIAL_TYPE_CHECK_
	static_assert(std::is_trivial_v<T>, "ksn::ppvector only works with trivial types. #define _KSN_PPVECTOR_NO_TRIVIAL_TYPE_CHECK_ to suppress this warning");
#endif // !_KSN_PPVECTOR_NO_TRIVIAL_TYPE_CHECK_

	size_t m_count, m_capacity;
	T* m_buffer;

	ppvector() noexcept
	{
		this->m_buffer = nullptr;
		this->m_count = this->m_capacity = 0;
	}
	ppvector(const ppvector<T>& r) noexcept
	{
		this->ppvector<T>::ppvector();
		if (this->reserve(r.m_capacity))
		{
			memcpy(this->m_buffer, r.m_buffer, r.m_count * sizeof(T));
			//this->m_capacity = r.m_capacity;
			this->m_count = r.m_count;
		}
	}
	ppvector(ppvector<T>&& r) noexcept
	{
		this->m_buffer = r.m_buffer;
		this->m_capacity = r.m_capacity;
		this->m_count = r.m_count;
		r.ppvector<T>::ppvector();
	}

	template<std::convertible_to<T> U>
	ppvector(const ppvector<U>& r) noexcept
	{
		this->ppvector<T>::ppvector();
		if (this->reserve(r.m_capacity))
		{
			for (this->m_count = 0; this->m_count < r.m_count; this->m_count++)
			{
				this->m_buffer[this->m_count] = T(r.m_buffer[this->m_count]);
			}
		}
	}
	template<std::convertible_to<T> U>
	ppvector(ppvector<U>&& r) noexcept
	{
		if constexpr (sizeof(T) <= sizeof(U))
		{
			//We can steal other's storage and do a conversion in-place
			this->m_buffer = r.m_buffer;
			this->m_capacity = r.m_capacity * sizeof(U) / sizeof(T);
		}
		else
		{
			this->ppvector<T>::ppvector();
			if (!this->reserve(r.m_capacity)) return;
		}
		for (this->m_count = 0; this->m_count < r.m_count; this->m_count++)
		{
			this->m_buffer[this->m_count] = T(r.m_buffer[this->m_count]);
		}
		if constexpr (sizeof(T) <= sizeof(U))
		{
			r.ppvector<U>::ppvector();
		}
	}

	bool reserve(size_t new_capacity) noexcept
	{
		if (this->m_capacity >= new_capacity) return true;

		void* memory = malloc(new_capacity * sizeof(T));
		if (memory == nullptr) return false;

		if (this->m_buffer) memcpy(memory, this->m_buffer, this->m_count * sizeof(T));
		::free(this->m_buffer);
		this->m_buffer = (T*)memory;
		this->m_capacity = new_capacity;
		return true;
	}
	bool reserve_more(size_t additional_capacity) noexcept
	{
		return this->reserve(this->m_capacity + additional_capacity);
	}

	template<std::convertible_to<T> U>
	bool push_back(const U& value) noexcept
	{
		if (!this->reserve_more(1)) return false;

		this->m_buffer[this->m_count++] = T(value);
		return true;
	}
};

_KSN_END



#endif //!_KSN_PPVECTOR_HPP_
