#ifndef _KSN_ITERATOR_HPP_
#define _KSN_ITERATOR_HPP_


#include <ksn/ksn.hpp>

#include <iterator>


_KSN_BEGIN


_KSN_DETAIL_BEGIN

template<class iterator_type, class STD_iterator_category>
struct reverse_iterator_base
{
	reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>);
	reverse_iterator_base(const iterator_type&) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
};

template<class iterator_type>
struct reverse_iterator_base<iterator_type, ::std::input_iterator_tag>
{
	using my_type = reverse_iterator_base<iterator_type, ::std::input_iterator_tag>;

	typename iterator_type::reference operator*() const;

	bool operator==(my_type other) const noexcept;
	bool operator!=(my_type other) const noexcept;


	iterator_type iter;

	reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>);
	reverse_iterator_base(const iterator_type&) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
};

template<class iterator_type>
struct reverse_iterator_base<iterator_type, ::std::output_iterator_tag>
{
	using my_type = reverse_iterator_base<iterator_type, ::std::output_iterator_tag>;

	typename iterator_type::reference operator*() const;

	bool operator==(my_type other) const noexcept;
	bool operator!=(my_type other) const noexcept;


	iterator_type iter;

	reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>);
	reverse_iterator_base(const iterator_type&) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
};

template<class iterator_type>
struct reverse_iterator_base<iterator_type, ::std::forward_iterator_tag> :
	public reverse_iterator_base<iterator_type, ::std::output_iterator_tag>
{
	using my_type = reverse_iterator_base<iterator_type, ::std::forward_iterator_tag>;


	reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>);
	reverse_iterator_base(const iterator_type&) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
};

template<class iterator_type>
struct reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag> :
	public reverse_iterator_base<iterator_type, ::std::forward_iterator_tag>
{
	using my_type = reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>;

	my_type& operator++();
	my_type& operator--();
	my_type operator++(int);
	my_type operator--(int);


	reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>);
	reverse_iterator_base(const iterator_type&) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
};

template<class iterator_type>
struct reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag> :
	public reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>
{
	using my_type = reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>;

	my_type operator+(typename iterator_type::difference_type) const noexcept;
	my_type operator-(typename iterator_type::difference_type) const noexcept;

	my_type& operator+=(typename iterator_type::difference_type) noexcept;
	my_type& operator-=(typename iterator_type::difference_type) noexcept;

	bool operator<(my_type other);
	bool operator<=(my_type other);
	bool operator>(my_type other);
	bool operator>=(my_type other);

	reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>);
	reverse_iterator_base(const iterator_type&) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
};


_KSN_DETAIL_END

template<class iterator_type>
struct reverse_iterator :
	public detail::reverse_iterator_base<iterator_type, typename iterator_type::iterator_category>
{
	using my_type = reverse_iterator<iterator_type>;

	reverse_iterator() noexcept(std::is_nothrow_default_constructible_v<iterator_type>);
	reverse_iterator(const my_type& other) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
	reverse_iterator(my_type&& other) noexcept(std::is_nothrow_move_constructible_v<iterator_type>);
	reverse_iterator(const iterator_type&) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>);
	reverse_iterator(iterator_type&& other) noexcept(std::is_nothrow_move_constructible_v<iterator_type>);

	operator iterator_type() const;
};


_KSN_END





_KSN_BEGIN

_KSN_DETAIL_BEGIN

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::input_iterator_tag>::reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>)
{
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::input_iterator_tag>::reverse_iterator_base(const iterator_type& x) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) :
	iter(x)
{
}



template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::output_iterator_tag>::reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>)
{
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::output_iterator_tag>::reverse_iterator_base(const iterator_type& x) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) :
	iter(x)
{
}



template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::forward_iterator_tag>::reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>)
{
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::forward_iterator_tag>::reverse_iterator_base(const iterator_type& x) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) :
	reverse_iterator_base<iterator_type, std::output_iterator_tag>(x)
{
}



template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>::reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>)
{
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>::reverse_iterator_base(const iterator_type& x) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) :
	reverse_iterator_base<iterator_type, std::forward_iterator_tag>(x)
{
}



template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>::reverse_iterator_base() noexcept(std::is_nothrow_default_constructible_v<iterator_type>)
{
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>::reverse_iterator_base(const iterator_type& x) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) :
	reverse_iterator_base<iterator_type, std::bidirectional_iterator_tag>(x)
{
	this->iter = this->iter;
}



template<class iterator_type>
typename iterator_type::reference reverse_iterator_base<iterator_type, ::std::input_iterator_tag>::operator*() const
{
	return *(this->iter);
}

template<class iterator_type>
bool reverse_iterator_base<iterator_type, ::std::input_iterator_tag>::operator==(my_type other) const noexcept
{
	return this->iter == other.iter;
}

template<class iterator_type>
bool reverse_iterator_base<iterator_type, ::std::input_iterator_tag>::operator!=(my_type other) const noexcept
{
	return this->iter != other.iter;
}



template<class iterator_type>
typename iterator_type::reference reverse_iterator_base<iterator_type, ::std::output_iterator_tag>::operator*() const
{
	return *(this->iter);
}

template<class iterator_type>
bool reverse_iterator_base<iterator_type, ::std::output_iterator_tag>::operator==(my_type other) const noexcept
{
	return this->iter == other.iter;
}

template<class iterator_type>
bool reverse_iterator_base<iterator_type, ::std::output_iterator_tag>::operator!=(my_type other) const noexcept
{
	return this->iter != other.iter;
}



template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>& reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>::operator++()
{
	--(this->iter);
	return *(reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>*)this;
	//Thanks to IntelliSense real-time error checking bugs on partial member functions initialization
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>& reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>::operator--()
{
	++(this->iter);
	return *(reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>*)this;
	//Thanks to IntelliSense real-time error checking bugs on partial member functions initialization
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag> reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>::operator++(int)
{
	auto copy(*this);
	--(this->iter);
	return *(reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>*)&copy;
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag> reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>::operator--(int)
{
	auto copy(*this);
	++(this->iter);
	return *(reverse_iterator_base<iterator_type, ::std::bidirectional_iterator_tag>*)&copy;
}



template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>
	reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>::operator+
	(typename iterator_type::difference_type n) const noexcept
{
	my_type result;
	result.iter = this->iter - n;
	return result;
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>::operator-
(typename iterator_type::difference_type n) const noexcept
{
	my_type result;
	result.iter = this->iter + n;
	return result;
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>&
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>::operator+=
(typename iterator_type::difference_type n) noexcept
{
	this->iter -= n;
	return *(reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>*)this;
}

template<class iterator_type>
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>&
reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>::operator-=
(typename iterator_type::difference_type n) noexcept
{
	this->iter += n;
	return *(reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>*)this;
}

template<class iterator_type>
bool reverse_iterator_base<iterator_type, ::std::random_access_iterator_tag>::operator<(my_type other)
{
	return this->iter > other.iter;
}

_KSN_DETAIL_END

template<class iterator_type>
reverse_iterator<iterator_type>::reverse_iterator() noexcept(std::is_nothrow_default_constructible_v<iterator_type>) :
	detail::reverse_iterator_base<iterator_type, typename iterator_type::iterator_category>()
{

}

template<class iterator_type>
reverse_iterator<iterator_type>::reverse_iterator(const my_type& other) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) :
	detail::reverse_iterator_base<iterator_type, typename iterator_type::iterator_category>(other)
{

}

template<class iterator_type>
reverse_iterator<iterator_type>::reverse_iterator(my_type&& other) noexcept(std::is_nothrow_move_constructible_v<iterator_type>) :
	detail::reverse_iterator_base<iterator_type, typename iterator_type::iterator_category>(std::forward<iterator_type>(other.iter))
{

}

template<class iterator_type>
reverse_iterator<iterator_type>::reverse_iterator(const iterator_type& other) noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) :
	detail::reverse_iterator_base<iterator_type, typename iterator_type::iterator_category>(other)
{

}

template<class iterator_type>
reverse_iterator<iterator_type>::reverse_iterator(iterator_type&& other) noexcept(std::is_nothrow_move_constructible_v<iterator_type>) :
	detail::reverse_iterator_base<iterator_type, typename iterator_type::iterator_category>(std::forward<iterator_type>(other))
{

}

template<class iterator_type>
reverse_iterator<iterator_type>::operator iterator_type() const
{
	return this->iter;
}

_KSN_END



#endif //!_KSN_ITERATOR_HPP_