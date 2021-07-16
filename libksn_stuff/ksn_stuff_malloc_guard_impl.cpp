
#include <ksn/stuff.hpp>

#include <stdlib.h>





_KSN_BEGIN


malloc_guard::malloc_guard() noexcept
	: m_data(nullptr), m_count(0), m_capacity(0)
{
}

malloc_guard::malloc_guard(malloc_guard&& rhs) noexcept
	: m_data(rhs.m_data), m_count(rhs.m_count), m_capacity(rhs.m_capacity)
{
	rhs.malloc_guard::malloc_guard();
}

bool malloc_guard::reserve(size_t n) noexcept
{
	if (this->m_capacity >= n) return true;

	uintptr_t* new_data = (uintptr_t*)malloc(n * sizeof(uintptr_t));
	if (new_data == nullptr) return false;

	if (this->m_data)
	{
		memcpy(new_data, this->m_data, this->m_count * sizeof(uintptr_t));
		::free(this->m_data);
	}

	this->m_data = new_data;
	this->m_capacity = n;
	return true;
}

bool malloc_guard::reserve_more(size_t add) noexcept
{
	return this->reserve(this->m_count + add);
}

void* malloc_guard::alloc(size_t sz) noexcept
{
	bool ok = this->reserve_more(1);
	if (!ok) return nullptr;

	void* memory = malloc(sz);
	if (memory == nullptr) return nullptr;

	this->m_data[this->m_count++] = (uintptr_t)memory;
	return memory;
}

void malloc_guard::free(void* p) noexcept
{
	::free(p);
	for (size_t i = 0; i < this->m_count; ++i)
	{
		if (this->m_data[i] == (uintptr_t)p)
		{
			this->m_data[i] = 0;
			return;
		}
	}
}

malloc_guard::~malloc_guard() noexcept
{
	for (size_t i = 0; i < this->m_count; ++i)
	{
		::free((void*)this->m_data[i]);
	}
}


malloc_guard& malloc_guard::operator=(malloc_guard&& x) noexcept
{
	std::swap(this->m_capacity, x.m_capacity);
	std::swap(this->m_count, x.m_count);
	std::swap(this->m_data, x.m_data);
	return *this;
}


_KSN_END
