
#ifndef _KSN_OPENCL_KERNEL_TESTER_HPP_
#define _KSN_OPENCL_KERNEL_TESTER_HPP_

#include <ksn/ppvector.hpp>
//#include <ksn/stuff.hpp>
#include <algorithm>
#include <execution>

#define __kernel
#define __global
#define __local
#define __private

namespace ksn_opencl_kernel_tester
{
	_KSN_DETAIL_BEGIN

	extern ksn::ppvector<size_t> global_id, global_size, local_id, local_size, global_end, local_end;

	_KSN_DETAIL_END;


	size_t get_global_id(size_t dim) noexcept;
	size_t get_local_id(size_t dim) noexcept;
	size_t get_global_size(size_t dim) noexcept;
	size_t get_local_size(size_t dim) noexcept;

	//May fail to memory allocation failure
	template<class callable_t, class... args_t>
	bool call_kernel(size_t work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size, callable_t&& callable, args_t&&... args) noexcept
	{
		using namespace detail;

		if (!global_id.reserve(work_dim)) return false;
		if (!global_size.reserve(work_dim)) return false;
		if (!global_end.reserve(work_dim)) return false;
		if (!local_id.reserve(work_dim)) return false;
		if (!local_size.reserve(work_dim)) return false;
		if (!local_end.reserve(work_dim)) return false;

		memcpy(global_id.m_buffer, global_work_offset, sizeof(size_t) * work_dim);
		memcpy(global_size.m_buffer, global_work_size, sizeof(size_t) * work_dim);
		memset(local_id.m_buffer, 0, sizeof(size_t) * work_dim);
		if (local_size != nullptr)
			memcpy(local_size.m_buffer, local_work_size, sizeof(size_t) * work_dim);
		else
			std::fill(local_size.begin(), local_size.end(), size_t(1));

		std::transform(std::execution::par_unseq, global_work_offset, global_work_offset + work_dim, global_work_size, global_end.m_buffer, [](size_t first, size_t count) { return first + count; });
		std::copy(std::execution::par_unseq, local_work_size, local_work_size + work_dim, local_end.m_buffer);
		bool stop = false;
		while (1)
		{
			if (std::mismatch(std::execution::par_unseq, global_id.begin(), global_id.end(), global_end.begin()).first == global_id.end()) break;

			std::invoke(std::forward<callable_t>(callable), std::forward<args_t>(args)...);

			for (size_t i = 0; i < work_dim; ++i)
			{
				if (global_id[i] < global_end[i]) ++global_id[i];
				if (global_id[i] != global_end[i]) break;
				global_id[i] = global_start[i];
			}
		}
	}
}



#endif //!_KSN_OPENCL_KERNEL_TESTER_HPP_
