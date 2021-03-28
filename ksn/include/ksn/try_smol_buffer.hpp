
#ifndef _KSN_TRY_SMOL_BUFFER_HPP_
#define _KSN_TRY_SMOL_BUFFER_HPP_


#include <ksn/ksn.hpp>

#include <type_traits>
//#include <alig>

#include <string.h>



_KSN_BEGIN


template<typename T, size_t N>
struct try_smol_buffer
{
	std::aligned_storage_t< N * sizeof(T), alignof(T)> smol;
	T* arr;
	size_t cap;
	
	try_smol_buffer() noexcept
		: arr((T*)&smol), cap(0) {}
	try_smol_buffer(size_t size) noexcept
	{
		this->try_smol_buffer<T, N>::try_smol_buffer();

		this->cap = size;
		if (size > N)
		{
			this->arr = (T*)malloc(size * sizeof(T));
		}
	}
	template<typename T1, size_t N1>
	try_smol_buffer(const try_smol_buffer<T1, N1>& r) noexcept
	{
		this->try_smol_buffer<T, N>::try_smol_buffer(r.cap);
		memcpy(this->arr, r.arr, sizeof(T) * this->cap);
	}
	try_smol_buffer(try_smol_buffer<T, N>&& r) noexcept
	{
		this->try_smol_buffer<T, N>::try_smol_buffer();

		if (r.cap > N)
		{
			std::swap(this->smol, r.smol);
		}
		std::swap(this->arr, r.arr);
		std::swap(this->cap, r.cap);
	}

	~try_smol_buffer() noexcept
	{
		if (this->cap > N) free(this->arr);
	}

	bool resize(size_t n) noexcept
	{
		if (n <= N)
		{
			if (this->cap > N)
			{
				free(this->arr);
				this->arr = (T*)&smol;
			}
		}
		else
		{
			if (this->cap <= N)
			{
				this->arr = (T*)malloc(N * sizeof(T));
				this->cap = 0;
				if (this->arr == nullptr) return false;
			}
		}
		this->cap = n;
		return true;
	}
	bool resize_add(size_t add) noexcept
	{
		return this->resize(this->cap + add);
	}

	T* data() noexcept
	{
		return this->arr;
	}
	const T* data() const noexcept
	{
		return this->arr;
	}
	T& operator[](size_t n) noexcept
	{
		return arr[n];
	}
	const T& operator[](size_t n) const noexcept
	{
		return arr[n];
	}

	operator T*() noexcept
	{
		return this->arr;
	}
	operator const T*() const noexcept
	{
		return this->arr;
	}
};


_KSN_END


#endif //!_KSN_TRY_SMOL_BUFFER_HPP_
