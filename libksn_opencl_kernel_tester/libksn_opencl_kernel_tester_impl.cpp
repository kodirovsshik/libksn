
#include <ksn/opencl_kernel_tester.hpp>
#include <ksn/stuff.hpp>

#include <numeric>

namespace ksn_opencl_kernel_tester
{
	_KSN_DETAIL_BEGIN
	
	ksn::ppvector<size_t> global_id, global_size, local_id, local_size;
	
	_KSN_DETAIL_END;

	size_t get_global_id(size_t dim) noexcept
	{
		return detail::global_id[dim];
	}
	size_t get_local_id(size_t dim) noexcept
	{
		return detail::local_id[dim];
	}
	size_t get_global_size(size_t dim) noexcept
	{
		return detail::global_size[dim];
	}
	size_t get_local_size(size_t dim) noexcept
	{
		return detail::local_size[dim];
	}
}
