
#ifndef _KSN_PPVECTOR_HPP_
#define _KSN_PPVECTOR_HPP_


#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>

#include <stdlib.h>
#include <string.h>

#include <iterator>
#include <type_traits>



_KSN_BEGIN

//Poor programmer's vector
//std::vector class for trivial types (only!!)

template<typename T> requires(!std::is_const_v<T> && !std::is_volatile_v<T>)
struct ppvector
{
	T* m_buffer;
	size_t m_count, m_capacity;


	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	

	~ppvector() noexcept
	{
		this->free();
	}
	ppvector() noexcept
	{
		this->m_buffer = nullptr;
		this->m_capacity = this->m_count = 0;
	}
	ppvector(const ppvector<T>& r) noexcept
	{
		this->ppvector<T>::ppvector();
		if (this->reserve(r.m_capacity))
		{
			memcpy(this->m_buffer, r.m_buffer, r.m_count * sizeof(T));
			//this->m_capacity = r.m_capacity; //Done by this->reserve(...)
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
	ppvector(const std::initializer_list<T>& list) noexcept
	{
		this->ppvector<T>::ppvector();
		if (this->reserve(list.size()))
		{
			memcpy(this->m_buffer, list.begin(), list.size() * sizeof(T));
			this->m_count = list.size();
		}
	}

	ppvector& operator=(const ppvector<T>& r) noexcept
	{
		if (this->reserve(r.m_count))
		{
			this->m_count = r.m_count;
			memcpy(this->m_buffer, r.m_buffer, sizeof(T) * r.m_count);
		}
		return *this;
	}
	ppvector& operator=(ppvector<T>&& r) noexcept
	{
		std::swap(this->m_buffer, r.m_buffer);
		std::swap(this->m_capacity, r.m_capacity);
		std::swap(this->m_count, r.m_count);
		return *this;
	}

	template<typename U> requires(std::convertible_to<U, T>)
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
	template<typename U> requires(std::convertible_to<U, T>)
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
		if constexpr
			(
				(std::is_integral_v<T> && std::is_integral_v<U> && sizeof(T) == sizeof(U)) ||
				(std::is_floating_point_v<T> && std::is_floating_point_v<U> && sizeof(T) == sizeof(U)) ||
				(std::is_pointer_v<T> && std::is_pointer_v<U>)
			)
		{
			//then we don't have to do anything about conversion
		}
		else
		{
			for (this->m_count = 0; this->m_count < r.m_count; this->m_count++)
			{
				this->m_buffer[this->m_count] = T(r.m_buffer[this->m_count]);
			}
		}
		if constexpr (sizeof(T) <= sizeof(U))
		{
			r.ppvector<U>::ppvector();
		}
	}



	bool operator==(const ppvector<T>& other) const noexcept
	{
		if (this->count != other.m_count) return false;
		return ::memcmp(this->m_buffer, other.m_buffer, sizeof(T) * this->m_count) == 0;
	}
	bool operator!=(const ppvector<T>& other) const noexcept
	{
		return !(*this == other);
	}



	T& operator[](size_t ndx) noexcept
	{
		return this->m_buffer[ndx];
	}
	const T& operator[](size_t ndx) const noexcept
	{
		return this->m_buffer[ndx];
	}
	T& at(size_t ndx) noexcept
	{
		return this->m_buffer[ndx];
	}
	const T& at(size_t ndx) const noexcept
	{
		return this->m_buffer[ndx];
	}

	T* data() noexcept
	{
		return this->m_buffer;
	}
	const T* data() const noexcept
	{
		return this->m_buffer;
	}



	iterator begin() noexcept
	{
		return iterator(this->m_buffer);
	}
	const_iterator begin() const noexcept
	{
		return iterator(this->m_buffer);
	}
	const_iterator cbegin() const noexcept
	{
		return iterator(this->m_buffer);
	}
	
	reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(this->m_buffer + this->m_count - 1);
	}
	const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(this->m_buffer + this->m_count - 1);
	}
	const_reverse_iterator crbegin() const noexcept
	{
		return const_reverse_iterator(this->m_buffer + this->m_count - 1);
	}
	
	iterator end() noexcept
	{
		return iterator(this->m_buffer + this->m_count);
	}
	const_iterator end() const noexcept
	{
		return iterator(this->m_buffer + this->m_count);
	}
	const_iterator cend() const noexcept
	{
		return iterator(this->m_buffer + this->m_count);
	}

	reverse_iterator rend() noexcept
	{
		return reverse_iterator(this->m_buffer - 1);
	}
	const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(this->m_buffer - 1);
	}
	const_reverse_iterator crend() const noexcept
	{
		return const_reverse_iterator(this->m_buffer - 1);
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

	template<typename U> requires(std::convertible_to<U, T>)
	bool push_back(const U& value) noexcept
	{
		if (!this->reserve_more(1)) return false;

		this->m_buffer[this->m_count++] = T(value);
		return true;
	}

	bool shrink() noexcept
	{
		if (this->m_capacity >= this->m_count)
		{
			if ((this->m_buffer = (T*)realloc(this->m_buffer, this->m_count * sizeof(T))) == nullptr)
			{
				void* new_mem = malloc(this->m_count * sizeof(T));
				if (new_mem = nullptr) return false;

				memcpy(new_mem, this->m_buffer, this->m_count * sizeof(T));
				::free(this->m_buffer);
				this->m_buffer = (T*)new_mem;
			}

			this->m_capacity = this->m_count;
			return true;
		}
	}

	void free() noexcept
	{
		::free(this->m_buffer);
		this->m_capacity = this->m_count = 0;
	}

	size_t size() const noexcept
	{
		return this->m_count;
	}
	size_t count() const noexcept
	{
		return this->m_count;
	}
	size_t capacity() const noexcept
	{
		return this->m_capacity;
	}
};

_KSN_END



#endif //!_KSN_PPVECTOR_HPP_
