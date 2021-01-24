#ifndef _KSN_TREE_HPP_
#define _KSN_TREE_HPP_

#include <ksn/ksn.hpp>

#include <memory>



_KSN_BEGIN



_KSN_DETAIL_BEGIN

template <class value_type, class alloc = std::allocator<value_type>>
class tree_base
{
private:
	struct node
	{
		value_type value;
		node *left, *right, *parent;
	};

private:
	node* proot;
};

_KSN_DETAIL_END



_KSN_END



#endif //_KSN_TREE_HPP_