#ifndef _KSN_VECTOR_HPP_
#define _KSN_VECTOR_HPP_


#include <ksn/ksn.hpp>
#include <ksn/iterator.hpp>
#include <ksn/function.hpp>

#include <vector>


#pragma warning(disable : 5051)


_KSN_BEGIN



_KSN_DETAIL_BEGIN

template<class Data, class Ptr, class Ref>
struct vector_iterator_base :
	public std::iterator<std::random_access_iterator_tag, Data, ptrdiff_t, Ptr, Ref>
{
	Ptr m_p_current;

	using my_type = vector_iterator_base <Data, Ptr, Ref>;

	my_type& operator++() noexcept;
	my_type& operator--() noexcept;

	my_type operator++(int) noexcept;
	my_type operator--(int) noexcept;

	my_type& operator+=(ptrdiff_t) noexcept;
	my_type& operator-=(ptrdiff_t) noexcept;

	Ref operator*() const;
	Ref operator[](ptrdiff_t) const;

	bool operator<(my_type other) const noexcept;
	bool operator<=(my_type other) const noexcept;
	bool operator>(my_type other) const noexcept;
	bool operator>=(my_type other) const noexcept;
	bool operator==(my_type other) const noexcept;
	bool operator!=(my_type other) const noexcept;
};

_KSN_DETAIL_END



template<class vector_type>
struct vector_iterator :
	public detail::vector_iterator_base<
	typename vector_type::value_type,
	typename vector_type::pointer,
	typename vector_type::reference
	>
{
	using my_type = vector_iterator<vector_type>;

	vector_iterator() noexcept;
	vector_iterator(const vector_iterator<vector_type>&) noexcept;
	vector_iterator(vector_iterator<vector_type>&&) noexcept;
	
	my_type& operator=(const my_type& other);
};

template<class vector_type>
struct vector_const_iterator :
	public detail::vector_iterator_base<
	typename vector_type::value_type,
	typename vector_type::const_pointer,
	typename vector_type::const_reference
	>
{
	vector_const_iterator() noexcept;
	vector_const_iterator(const vector_const_iterator<vector_type>&) noexcept;
	vector_const_iterator(vector_const_iterator<vector_type>&&) noexcept;
	vector_const_iterator(const vector_iterator<vector_type>&) noexcept;
	vector_const_iterator(vector_iterator<vector_type>&&) noexcept;
};


template<class Type, class Alloc = std::allocator<Type>>
struct vector
{
private:

	//Initial size of every created ksn vector (except the ctor with capacity)
	static size_t sg_init_size;

public:
	using my_type = vector<Type, Alloc>;
	using my_std_type = std::vector<Type, Alloc>;
	using value_type = Type;
	using allocator_type = Alloc;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using reference = Type &;
	using const_reference = const Type &;
	using pointer = typename std::allocator_traits<Alloc>::pointer;
	using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
	using iterator = vector_iterator<my_type>;
	using const_iterator = vector_const_iterator<my_type>;
	using reverse_iterator = ksn::reverse_iterator<iterator>;
	using const_reverse_iterator = ksn::reverse_iterator<const_iterator>;





	pointer m_ptr;
	size_t m_size;
	size_t m_capacity;
	Alloc m_allocator;



	static __forceinline void set_init_size(size_t) noexcept;
	static __forceinline size_t get_init_size() noexcept;

	~vector();

	//Default constructor
	vector();

	//Default constructor
	explicit vector(const Alloc&);



	//Constructor with start capacity
	vector(size_t default_capacity);

	//Constructor with start capacity
	vector(size_t default_capacity, const Alloc&);



	//Constructs a vector with N copies of specified element
	vector(size_t elements_count, const Type& element);

	//Constructs a vector with N copies of specified element
	vector(size_t elements_count, const Type& element, const Alloc&);



	//Constructs a vector from an initializer list
	vector(std::initializer_list<Type>);

	//Constructs a vector from an initializer list
	vector(std::initializer_list<Type>, const Alloc&);



	//Constructs a vector from an initializer list of other type
	template<class _Type>
	vector(std::initializer_list<_Type>);

	//Constructs a vector from an initializer list of other type
	template<class _Type>
	vector(std::initializer_list<_Type>, const Alloc&);



	//Constructs a vector with elements in range [begin, end)
	template<typename ForwardIterator>
	vector(ForwardIterator begin, ForwardIterator end);

	//Constructs a vector with elements in range [begin, end)
	template<typename ForwardIterator>
	vector(ForwardIterator begin, ForwardIterator end, const Alloc&);



	//Constructs a vector from a STL vector
	vector(const my_std_type&);

	//Constructs a vector from a STL vector
	vector(const my_std_type&, const Alloc&);

	//Constructs a vector by moving a STL vector
	vector(my_std_type&&);

	//Constructs a vector by moving a STL vector
	vector(my_std_type&&, const Alloc&);



	//Constructs a vector from a STL vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(const std::vector<_Type, _Alloc>&);

	//Constructs a vector from a STL vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(const std::vector<_Type, _Alloc>&, const Alloc&);

	//Constructs a vector by moving a STL vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(std::vector<_Type, _Alloc>&&);

	//Constructs a vector by moving a STL vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(std::vector<_Type, _Alloc>&&, const Alloc&);



	//Constructs a vector from another vector
	vector(const my_type&);

	//Constructs a vector from another vector
	vector(const my_type&, const Alloc&);

	//Constructs a vector by moving another vector
	vector(my_type&&);

	//Constructs a vector by moving another vector
	vector(my_type&&, const Alloc&);



	//Constructs a vector from a vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(const vector<_Type, _Alloc>&);

	//Constructs a vector from a vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(const vector<_Type, _Alloc>&, const Alloc&);

	//Constructs a vector by moving a vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(vector<_Type, _Alloc>&&);

	//Constructs a vector by moving a vector of other data/allocator type
	template<class _Type, class _Alloc>
	vector(vector<_Type, _Alloc>&&, const Alloc&);





	my_type& operator=(const my_type&);
	my_type& operator=(my_type&&);

	template<class _Type, class _Alloc>
	my_type& operator=(const vector<_Type, _Alloc>&);
	template<class _Type, class _Alloc>
	my_type& operator=(vector<_Type, _Alloc>&&);



	my_type& operator=(const my_std_type&);
	my_type& operator=(my_std_type&&);

	template<class _Type, class _Alloc>
	my_type& operator=(const std::vector<_Type, _Alloc>&);
	template<class _Type, class _Alloc>
	my_type& operator=(std::vector<_Type, _Alloc>&&);





	void assign();

	void assign(size_t count);
	void assign(size_t count, const value_type& element);

	void assign(std::initializer_list<value_type> list);
	template<class ForwardIterator>
	void assign(ForwardIterator begin, ForwardIterator end);

	void assign(const my_std_type& other);
	void assign(my_std_type&& other);

	template<class _Type, class _Alloc>
	void assign(const std::vector<_Type, _Alloc>& other);
	template<class _Type, class _Alloc>
	void assign(std::vector<_Type, _Alloc>&& other);

	void assign(const my_type& other);
	void assign(my_type&& other);

	template<class _Type, class _Alloc>
	void assign(const vector<_Type, _Alloc>& other);
	template<class _Type, class _Alloc>
	void assign(vector<_Type, _Alloc>&& other);

	allocator_type get_allocator() const;

	reference at(difference_type);
	const_reference at(difference_type) const;

	reference operator[](difference_type);
	const_reference operator[](difference_type) const;

	reference front();
	const_reference front() const;

	reference back();
	const_reference back() const;

	value_type* data() noexcept;
	const value_type* data() const noexcept;

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	const_iterator cbegin() const noexcept;

	iterator end() noexcept;
	const_iterator end() const noexcept;
	const_iterator cend() const noexcept;

	reverse_iterator rbegin() noexcept;
	const_reverse_iterator rbegin() const noexcept;
	const_reverse_iterator crbegin() const noexcept;

	reverse_iterator rend() noexcept;
	const_reverse_iterator rend() const noexcept;
	const_reverse_iterator crend() const noexcept;

	bool empty() const noexcept;

	size_type size() const noexcept;

	size_type max_size() const noexcept;

	void reserve(size_type new_capacity);

	size_type capacity() const noexcept;

	void shrink_to_fit();

	void clear() noexcept;

	iterator insert(const_iterator pos, const value_type& value);
	iterator insert(const_iterator pos, value_type&& value);
	iterator insert(const_iterator pos, size_type count, const value_type& value);
	template<class InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last);
	iterator insert(const_iterator pos, std::initializer_list<value_type> ilist);

	template<class... Args>
	iterator emplace(const_iterator pos, Args&& ...args);

	iterator erase(const_iterator pos);
	iterator erase(const_iterator first, const_iterator last);
	//If positions vector is not sorted it causes an undefined behaviour
	//it is your duty to make it sorted
	void erase(const vector<size_t>& sorted_positions);

	void push_back(const value_type& value);
	void push_back(value_type&& value);

	void push_front(const value_type& value);
	void push_front(value_type&& value);

	template< class... Args >
	reference emplace_back(Args&& ... args);
	template< class... Args >
	reference emplace_front(Args&& ... args);
	template< class... Args >
	reference emplace(iterator where, Args&& ... args);

	void pop_back();

	void resize(size_type count);
	void resize(size_type count, const value_type& value);

	void swap(my_type& other)
#if _KSN_CPP_VER >= 201700L
		noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value
			|| std::allocator_traits<Allocator>::is_always_equal::value)
#endif
		;
	template<class _Type, class _Alloc>
	void swap(const vector<_Type, _Alloc>& other);

	bool operator==(const my_type& other) const noexcept;
	bool operator!=(const my_type& other) const noexcept;
	bool operator>=(const my_type& other) const noexcept;
	bool operator<=(const my_type& other) const noexcept;
	bool operator>(const my_type& other) const noexcept;
	bool operator<(const my_type& other) const noexcept;


private: //internal stuff
	void allocate_storage(size_t);
	void reallocate_storage(size_t);

	void init_storage(size_t, size_t, const value_type&);
	template<class forward_iterator>
	void init_storage(std::true_type memcpy_available, forward_iterator, forward_iterator, size_t = 0);
	template<class forward_iterator>
	void init_storage(std::false_type memcpy_available, forward_iterator, forward_iterator, size_t = 0);

	void destruct_storage(size_t, size_t);
	void deallocate_storage();
};

_KSN_END


_STD_BEGIN

template<class T1, class A1, class T2, class A2>
void swap(ksn::vector<T1, A1>& v1, ksn::vector<T2, A2>& v2);

template<class Type, class Alloc, class U>
void erase(ksn::vector<Type, Alloc>& vector, U&& val);

template<class Type, class Alloc, class Predicate>
void erase_if(ksn::vector<Type, Alloc>& vector, Predicate predicate);

_STD_END





///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////





//
_KSN_BEGIN



template<class T1, class A1, class T2, class A2>
void swap(ksn::vector<T1, A1>& v1, ksn::vector<T2, A2>& v2)
{
	v1.swap(v2);
}

template<class Type, class Alloc, class U>
void erase(ksn::vector<Type, Alloc>& vector, U&& val)
{
	ksn::vector<size_t> erases(8);
	
	size_t current = 0, last = vector.m_size();

	while (current < last)
	{
		if (vector[current] == val)
			erases.push_back(current);
		++current;
	}

	vector.erase(erases);
}

template<class Type, class Alloc, class Predicate>
void erase_if(ksn::vector<Type, Alloc>& vector, Predicate predicate);





template<class Type, class Alloc>
static void vector<Type, Alloc>::set_init_size(size_t n) noexcept
{
	if (n)
	{
		my_type::sg_init_size = n;
	}
}
template<class Type, class Alloc>
static size_t vector<Type, Alloc>::get_init_size() noexcept
{
	return my_type::sg_init_size;
}





template<class Type, class Alloc>
size_t vector<Type, Alloc>::sg_init_size = 8;

template<class Type, class Alloc>
void vector<Type, Alloc>::allocate_storage(size_t n)
{
	if (n == 0)
	{
		n = my_type::sg_init_size;
	}

	this->m_capacity = n;
	this->m_ptr = std::allocator_traits<Alloc>::allocate(this->m_allocator, n);
}

template<class Type, class Alloc>
void vector<Type, Alloc>::reallocate_storage(size_t n)
{
	if (n <= this->m_capacity)
	{
		return;
	}

	pointer p_new = std::allocator_traits<Alloc>::allocate(this->m_allocator, n);
	memcpy(
		convert_or_forward<Type*>(p_new, _STD addressof(*p_new)),
		convert_or_forward<Type*>(&*(this->m_ptr), _STD addressof(*m_ptr)),
		sizeof(Type) * n
	);
	std::allocator_traits<Alloc>::deallocate(this->m_allocator, this->m_ptr, this->m_capacity);
	this->m_ptr = p_new;
	this->m_capacity = n;
}



template<class Type, class Alloc>
void vector<Type, Alloc>::init_storage(size_t from, size_t to, const value_type& val)
{
	while (from < to)
	{
		std::allocator_traits<Alloc>::construct(this->m_allocator, this->m_ptr + from++, val);
	}
}

template<class Type, class Alloc>
template<class forward_iterator>
void vector<Type, Alloc>::init_storage(std::false_type, forward_iterator first, forward_iterator last, size_t from)
{
	pointer current = this->m_ptr + from;
	while (first < last)
	{
		std::allocator_traits<Alloc>::construct(this->m_allocator, current++, *first++);
	}
}

template<class Type, class Alloc>
template<class forward_iterator>
void vector<Type, Alloc>::init_storage(std::true_type, forward_iterator first, forward_iterator last, size_t from)
{
	memcpy(
		convert_or_forward<Type*>(&*(this->m_ptr), _STD addressof(*this->m_ptr)) + from,
		convert_or_forward<Type*>(&*first, _STD addressof(*first)),
		sizeof(Type) * std::distance(first, last)
	);
}

template<class Type, class Alloc>
void vector<Type, Alloc>::destruct_storage(size_t from, size_t to)
{
	while (from < to)
	{
		std::allocator_traits<Alloc>::destroy(this->m_allocator, this->m_ptr + from++);
	}
}

template<class Type, class Alloc>
void vector<Type, Alloc>::deallocate_storage()
{
	std::allocator_traits<Alloc>::deallocate(this->m_allocator, this->m_ptr, this->m_capacity);
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector()
{
	this->allocate_storage(my_type::sg_init_size);
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(const Alloc& allocator) :
	m_allocator(allocator)
{
	this->allocate_storage(my_type::sg_init_size);
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector(size_t cap)
{
	this->allocate_storage(cap);
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(size_t cap, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->allocate_storage(cap);
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector(size_t elements, const Type& value)
{
	this->allocate_storage(elements);
	this->init_storage(0, elements, value);
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(size_t elements, const Type& value, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->allocate_storage(elements);
	this->init_storage(0, elements, value);
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector(std::initializer_list<Type> list)
{
	this->allocate_storage(list.size());
	this->init_storage(std::false_type(), list.begin(), list.end());
	this->m_size = list.size();
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(std::initializer_list<Type> list, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(list);
}



template<class Type, class Alloc>
template<class OType>
vector<Type, Alloc>::vector(std::initializer_list<OType> list)
{
	this->allocate_storage(list.size());
	this->init_storage(std::false_type(), list.begin(), list.end());
	this->m_size = list.size();
}

template<class Type, class Alloc>
template<class OType>
vector<Type, Alloc>::vector(std::initializer_list<OType> list, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(list);
}



template<class Type, class Alloc>
template<class ForwardIterator>
vector<Type, Alloc>::vector(ForwardIterator first, ForwardIterator last)
{
	this->allocate_storage(std::distance(first, last));
	this->init_storage(std::false_type(), first, last);
	this->m_size = std::distance(first, last);
}

template<class Type, class Alloc>
template<class ForwardIterator>
vector<Type, Alloc>::vector(ForwardIterator first, ForwardIterator last, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(first, last);
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector(const std::vector<Type, Alloc>& other) :
	m_allocator(other.get_allocator())
{
	this->my_type::vector(other.begin(), other.end());
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(const std::vector<Type, Alloc>& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector(std::vector<Type, Alloc>&& other) :
	m_allocator(other.get_allocator())
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(std::vector<Type, Alloc>&& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}



template<class Type, class Alloc>
template<class OType, class OAlloc>
vector<Type, Alloc>::vector(const std::vector<OType, OAlloc>& other) :
	m_allocator(convert_or_create<Alloc>(other.get_allocator()))
{
	this->my_type::vector(other.begin(), other.end());
}

template<class Type, class Alloc>
template<class OType, class OAlloc>
vector<Type, Alloc>::vector(const std::vector<OType, OAlloc>& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
}



template<class Type, class Alloc>
template<class OType, class OAlloc>
vector<Type, Alloc>::vector(std::vector<OType, OAlloc>&& other) :
	m_allocator(convert_or_create<Alloc>(other.get_allocator()))
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}

template<class Type, class Alloc>
template<class OType, class OAlloc>
vector<Type, Alloc>::vector(std::vector<OType, OAlloc>&& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector(const vector<Type, Alloc>& other) :
	m_allocator(other.m_allocator)
{
	this->my_type::vector(other.begin(), other.end());
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(const vector<Type, Alloc>& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
}



template<class Type, class Alloc>
vector<Type, Alloc>::vector(vector<Type, Alloc>&& other) :
	m_allocator(other.m_allocator)
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}

template<class Type, class Alloc>
vector<Type, Alloc>::vector(vector<Type, Alloc>&& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}



template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>::vector(const vector<_Type, _Alloc>& other) :
	m_allocator(convert_or_create<Alloc>(other.m_allocator))
{
	this->my_type::vector(other.begin(), other.end());
}

template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>::vector(const vector<_Type, _Alloc>& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
}



template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>::vector(vector<_Type, _Alloc>&& other) :
	m_allocator(convert_or_create<Alloc>(other.m_allocator))
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}

template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>::vector(vector<_Type, _Alloc>&& other, const Alloc& alloc) :
	m_allocator(alloc)
{
	this->my_type::vector(other.begin(), other.end());
	other.clear();
}

template<class Type, class Alloc>
vector<Type, Alloc>::~vector()
{
	this->destruct_storage(0, this->m_size);
	this->deallocate_storage();
}





template<class Type, class Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(const my_type& other)
{
	this->assign(other);
	return *this;
}

template<class Type, class Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(my_type&& other)
{
	this->assign(other);
	return *this;
}

template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(const vector<_Type, _Alloc>& other)
{
	this->assign(other);
	return *this;
}

template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(vector<_Type, _Alloc>&& other)
{
	this->assign(other);
	return *this;
}



template<class Type, class Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(const my_std_type& other)
{
	this->assign(other);
	return *this;
}

template<class Type, class Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(my_std_type&& other)
{
	this->assign(other);
	return *this;
}

template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(const std::vector<_Type, _Alloc>& other)
{
	this->assign(other);
	return *this;
}

template<class Type, class Alloc>
template<class _Type, class _Alloc>
vector<Type, Alloc>& vector<Type, Alloc>::operator=(std::vector<_Type, _Alloc>&& other)
{
	this->assign(other);
	return *this;
}





template<class Type, class Alloc>
void vector<Type, Alloc>::assign()
{
	this->destruct_storage(0, this->m_size);
	this->m_size = 0;
}

template<class Type, class Alloc>
void vector<Type, Alloc>::assign(size_t size)
{
	this->assign(size, Type());
}

template<class Type, class Alloc>
void vector<Type, Alloc>::assign(size_t size, const Type& value)
{ 
	this->destruct_storage(0, this->m_size);
	this->reallocate_storage(size);
	this->init_storage(0, size, value);
	this->m_size = size;
}

template<class Type, class Alloc>
void vector<Type, Alloc>::assign(const my_type& other) 
{
	this->destruct_storage(0, this->m_size);
	this->m_allocator = other.m_allocator;
	this->reallocate_storage(other.m_capacity);
	this->append(other.begin(), other.end());
}

template<class Type, class Alloc>
void vector<Type, Alloc>::assign(my_type&& other)
{
	this->destruct_storage(0, this->m_size);
	this->m_allocator = other.m_allocator;
	this->reallocate_storage(other.m_capacity);
	this->append(other.begin(), other.end());
	other.clear();
}



template<class Type, class Alloc>
typename vector<Type, Alloc>::iterator vector<Type, Alloc>::begin() noexcept
{
	iterator result;
	result.m_p_current = this->m_ptr;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_iterator vector<Type, Alloc>::begin() const noexcept
{
	const_iterator result;
	result.m_p_current = this->m_ptr;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_iterator vector<Type, Alloc>::cbegin() const noexcept
{
	const_iterator result;
	result.m_p_current = this->m_ptr;
	return result;
}



template<class Type, class Alloc>
typename vector<Type, Alloc>::iterator vector<Type, Alloc>::end() noexcept
{
	iterator result;
	result.m_p_current = this->m_ptr + this->m_size;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_iterator vector<Type, Alloc>::end() const noexcept
{
	iterator result;
	result.m_p_current = this->m_ptr + this->m_size;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_iterator vector<Type, Alloc>::cend() const noexcept
{
	iterator result;
	result.m_p_current = this->m_ptr + this->m_size;
	return result;
}



template<class Type, class Alloc>
typename vector<Type, Alloc>::reverse_iterator vector<Type, Alloc>::rbegin() noexcept
{
	reverse_iterator result;
	result.iter.m_p_current = this->m_ptr + this->m_size - 1;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_reverse_iterator vector<Type, Alloc>::rbegin() const noexcept
{
	const_reverse_iterator result;
	result.iter.m_p_current = this->m_ptr + this->m_size - 1;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_reverse_iterator vector<Type, Alloc>::crbegin() const noexcept
{
	const_reverse_iterator result;
	result.iter.m_p_current = this->m_ptr + this->m_size - 1;
	return result;
}



template<class Type, class Alloc>
typename vector<Type, Alloc>::reverse_iterator vector<Type, Alloc>::rend() noexcept
{
	reverse_iterator result;
	result.iter.m_p_current = this->m_ptr - 1;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_reverse_iterator vector<Type, Alloc>::rend() const noexcept
{
	const_reverse_iterator result;
	result.iter.m_p_current = this->m_ptr - 1;
	return result;
}

template<class Type, class Alloc>
typename vector<Type, Alloc>::const_reverse_iterator vector<Type, Alloc>::crend() const noexcept
{
	const_reverse_iterator result;
	result.iter.m_p_current = this->m_ptr - 1;
	return result;
}





_KSN_DETAIL_BEGIN

template<class Data, class Ptr, class Ref>
vector_iterator_base<Data, Ptr, Ref>& vector_iterator_base<Data, Ptr, Ref>::operator++() noexcept
{
	++(this->m_p_current);
	return *this;
}

template<class Data, class Ptr, class Ref>
vector_iterator_base<Data, Ptr, Ref>& vector_iterator_base<Data, Ptr, Ref>::operator--() noexcept
{
	--(this->m_p_current);
	return *this;
}

template<class Data, class Ptr, class Ref>
vector_iterator_base<Data, Ptr, Ref> vector_iterator_base<Data, Ptr, Ref>::operator++(int) noexcept
{
	auto copy(*this);
	++(this->m_p_current);
	return copy;
}

template<class Data, class Ptr, class Ref>
vector_iterator_base<Data, Ptr, Ref> vector_iterator_base<Data, Ptr, Ref>::operator--(int) noexcept
{
	auto copy(*this);
	--(this->m_p_current);
	return copy;
}

template<class Data, class Ptr, class Ref>
vector_iterator_base<Data, Ptr, Ref>& vector_iterator_base<Data, Ptr, Ref>::operator+=(ptrdiff_t n) noexcept
{
	this->m_p_current += n;
	return *this;
}

template<class Data, class Ptr, class Ref>
vector_iterator_base<Data, Ptr, Ref>& vector_iterator_base<Data, Ptr, Ref>::operator-=(ptrdiff_t n) noexcept
{
	this->m_p_current -= n;
	return *this;
}

template<class Data, class Ptr, class Ref>
Ref vector_iterator_base<Data, Ptr, Ref>::operator*() const
{
	return *this->m_p_current;
}

template<class Data, class Ptr, class Ref>
Ref vector_iterator_base<Data, Ptr, Ref>::operator[](ptrdiff_t n) const
{
	return this->m_p_current[n];
}

template<class Data, class Ptr, class Ref>
bool vector_iterator_base<Data, Ptr, Ref>::operator<(my_type other) const noexcept
{
	return this->m_p_current < other.m_p_current;
}

template<class Data, class Ptr, class Ref>
bool vector_iterator_base<Data, Ptr, Ref>::operator<=(my_type other) const noexcept
{
	return this->m_p_current <= other.m_p_current;
}

template<class Data, class Ptr, class Ref>
bool vector_iterator_base<Data, Ptr, Ref>::operator>(my_type other) const noexcept
{
	return this->m_p_current > other.m_p_current;
}

template<class Data, class Ptr, class Ref>
bool vector_iterator_base<Data, Ptr, Ref>::operator>=(my_type other) const noexcept
{
	return this->m_p_current >= other.m_p_current;
}

template<class Data, class Ptr, class Ref>
bool vector_iterator_base<Data, Ptr, Ref>::operator==(my_type other) const noexcept
{
	return this->m_p_current == other.m_p_current;
}

template<class Data, class Ptr, class Ref>
bool vector_iterator_base<Data, Ptr, Ref>::operator!=(my_type other) const noexcept
{
	return this->m_p_current != other.m_p_current;
}



_KSN_DETAIL_END

template<class vector_type>
vector_iterator<vector_type>::vector_iterator() noexcept
{
}

template<class vector_type>
vector_iterator<vector_type>::vector_iterator(const vector_iterator<vector_type>& other) noexcept
{
	this->m_p_current = other.m_p_current;
}

template<class vector_type>
vector_iterator<vector_type>::vector_iterator(vector_iterator<vector_type>&& other) noexcept
{
	this->m_p_current = other.m_p_current;
	other.m_p_current = decltype(other.m_p_current)();
}

template<class vector_type>
vector_iterator<vector_type>& vector_iterator<vector_type>::operator=(const vector_iterator<vector_type>& other)
{
	this->m_p_current = other.m_p_current;
	return *this;
}

_KSN_END



//todo: add size != 0 check to ksn::vector<...>::allocate_storage

#endif //!_KSN_VECTOR_HPP_
