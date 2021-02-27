
#include <ksn/opencl_kernel_tester.hpp>
#include <ksn/stuff.hpp>

#include <numeric>

namespace ksn_opencl_kernel_tester
{
	static ksn::ppvector<size_t> global_id, global_size, local_id, local_size;

	size_t get_global_id(size_t dim) noexcept
	{
		return global_id[dim];
	}
	size_t get_local_id(size_t dim) noexcept
	{
		return local_id[dim];
	}
	size_t get_global_size(size_t dim) noexcept
	{
		return global_size[dim];
	}
	size_t get_local_size(size_t dim) noexcept
	{
		return local_size[dim];
	}



	bool arguments_adapter_t::set_args_count(size_t n) noexcept
	{
		if (!this->args.reserve(n)) return false;
		for (auto& buffer : this->args)
		{
			buffer.ppvector<uint8_t>::ppvector();
		}
		return true;
	}

	bool arguments_adapter_t::set_arg(size_t index, const void* data, size_t size) noexcept
	{
		if (size >= this->args.size()) return false;

		if (!this->args[index].reserve(size)) return false;

		memcpy(this->args[index].m_buffer, data, size);
		this->args[index].m_count = size;
		return true;
	}

	arguments_adapter_t::~arguments_adapter_t() noexcept
	{
		for (auto& buffer : this->args)
		{
			buffer.~ppvector();
		}
	}


	extern "C" void _ksn_opencl_kernel_tester_call_wrapper(const void* callback, const void* arg_data, size_t size);

	bool call_kernel(void(__cdecl* p)(void), const arguments_adapter_t& args) noexcept
	{
		if (args.args.count() == 0)
		{
			(*p)();
			return true;
		}

		size_t memory_size = std::accumulate(args.args.begin(), args.args.end(), size_t(0), []
			(size_t val, const ksn::ppvector<uint8_t>& obj) -> size_t 
			{
				size_t size = obj.size();
				if (size < sizeof(void*)) size = sizeof(void*);
				return val + size;
			}
		);
	
		ksn::malloc_guard alloc;

		uint8_t* memory = (uint8_t*)alloc.alloc(memory_size);
		if (memory == nullptr) return false;

		size_t off = 0;
		for (const auto& buffer : args.args)
		{
			memcpy(memory + off, buffer.data(), buffer.size());
			size_t mem_diff = buffer.size();
			if (mem_diff < sizeof(void*)) mem_diff = sizeof(void*);
			off += mem_diff;
		}

		_ksn_opencl_kernel_tester_call_wrapper(p, memory, memory_size);
		return true;
	}
}
