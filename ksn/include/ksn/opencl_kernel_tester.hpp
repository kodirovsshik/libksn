
#ifndef _KSN_OPENCL_KERNEL_TESTER_HPP_
#define _KSN_OPENCL_KERNEL_TESTER_HPP_

#include <ksn/ppvector.hpp>

#define __kernel
#define __global
#define __local
#define __private

namespace ksn_opencl_kernel_tester
{
	size_t get_global_id(size_t dim) noexcept;
	size_t get_local_id(size_t dim) noexcept;
	size_t get_global_size(size_t dim) noexcept;
	size_t get_local_size(size_t dim) noexcept;

	struct arguments_adapter_t
	{
		ksn::ppvector<ksn::ppvector<uint8_t>> args;

		//May fail if can not allocate memory
		bool set_args_count(size_t) noexcept;
		//May fail if can not allocate memory or index >= value set by set_args_count (0 if never was set)
		bool set_arg(size_t index, const void* data, size_t size) noexcept;

		~arguments_adapter_t() noexcept;
	};

	bool call_kernel(void(__cdecl * casted_kernel)(void), const arguments_adapter_t&) noexcept;
}



#endif //!_KSN_OPENCL_KERNEL_TESTER_HPP_
