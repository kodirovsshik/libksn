
#ifndef _KSN_OPENCL_KERNEL_TESTER_HPP_
#define _KSN_OPENCL_KERNEL_TESTER_HPP_

#include <ksn/ppvector.hpp>

#define __kernel
#define __global
#define __local
#define __private

namespace ksn_opencl_kernel_tester
{
	_KSN_DETAIL_BEGIN

	extern ksn::ppvector<size_t> global_id, global_size, local_id, local_size;

	_KSN_DETAIL_END;


	size_t get_global_id(size_t dim) noexcept;
	size_t get_local_id(size_t dim) noexcept;
	size_t get_global_size(size_t dim) noexcept;
	size_t get_local_size(size_t dim) noexcept;

	template<class callable_t, class... args_t>
	bool call_kernel(size_t work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size, callable_t&& callable, args_t&&... args) noexcept
	{

	}
}



#endif //!_KSN_OPENCL_KERNEL_TESTER_HPP_
