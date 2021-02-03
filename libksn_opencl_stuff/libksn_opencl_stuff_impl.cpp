
#include <ksn/opencl_stuff.hpp>

#include <unordered_map>
#include <string_view>



#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/opencl.hpp>




#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)
#endif





_KSN_BEGIN



_KSN_CL_BEGIN



_KSN_DETAIL_BEGIN



extern const char* _iota_src;



static size_t _cl_platform = 0;
static uint64_t _cl_devices_mask = CL_DEVICE_TYPE_ALL;

static std::vector<std::pair<::cl::Platform, std::vector<::cl::Device>>> _opencl_impl_context;
//static std::unordered_map<std::string, size_t> _funcs_indexes;
static std::unordered_map<std::string, ::cl::Kernel> _func_kernels;
static const std::unordered_map<std::string_view, const char*> _func_sources = { {"iota", detail::_iota_src} };

static ::cl::Context* _context = nullptr;





struct __constructor
{
	cl_int result;

	__constructor(uint64_t device_types = CL_DEVICE_TYPE_ALL)
	{
#define ensure_ok() { if (result != CL_SUCCESS) return; } ((void)0)

		std::vector<::cl::Platform> platforms;

		result = ::cl::Platform::get(&platforms);
		ensure_ok();

		detail::_opencl_impl_context.reserve(platforms.size());
		detail::_opencl_impl_context.clear(); //In case it was already constructed
		detail::_func_kernels.clear();
		detail::_func_kernels.reserve(64);

		std::vector<::cl::Device> devices;
		for (const auto& platform : platforms)
		{
			result = platform.getDevices(device_types, &devices);
			ensure_ok();
			detail::_opencl_impl_context.emplace_back(platform, devices);
			devices.clear();
		}

		if (detail::_context != nullptr)
		{
			delete detail::_context;
		}

		detail::_context = new ::cl::Context(detail::_opencl_impl_context[detail::_cl_platform].second);

		this->result = 0;
	}

	operator int()
	{
		return this->result;
	}

} static __construct_result;





template<arithmetic T>
::cl::Kernel* get_kernel(const char* func_name)
{
	char parametrized_name[64];
	sprintf(parametrized_name, "%s<%s>", func_name, typeid(T).name());

	if (detail::_func_kernels.contains(parametrized_name))
	{
		return &detail::_func_kernels.at(parametrized_name);
	}

	char src_buffer[4096];
	int _ = _sprintf_p(src_buffer, 4096, detail::_func_sources.at(func_name), typeid(T).name());

	if (_ <= 0 || _ >= 4095) throw std::exception(parametrized_name);

	::cl::Program::Sources source{ src_buffer };
	::cl::Program prog(*detail::_context, source);
	cl_int status = prog.build("-cl-std=CL1.2");
	if (status != CL_SUCCESS)
	{
		if constexpr (!_KSN_HAS_EXCEPTIONS) return nullptr;

		auto v = prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>();
		std::vector<std::string> excp;
		excp.reserve(v.size());
		for (auto&& [d, s] : v)
		{
			excp.emplace_back(std::move(s));
		}
		_KSN_RAISE(excp);
	}

	auto&& [iter, success] = detail::_func_kernels.emplace(
		std::piecewise_construct,
		std::tuple{ parametrized_name },
		std::tuple{ prog, func_name }
	);
	if (!success) return nullptr;
	auto& [inserted_kernel_name, inserted_kernel] = *iter;
	return &inserted_kernel;
}



_KSN_DETAIL_END





template<arithmetic T>
void foo(T(*arr)[], size_t n, size_t m, T a, T b)
{
	cl_int err;
	auto& kernel = *detail::get_kernel<T>("iota");
	auto context = kernel.getInfo<CL_KERNEL_CONTEXT>(&err);
	T params[2] = { a, b };

	//::cl::Buffer arr_buf(context, CL_MEM_HOST_READ_ONLY | CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, n * m * sizeof(T), arr, &err);
	//::cl::Buffer param_buf(context, CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 2 * sizeof(T), params, &err);
	::cl::Buffer arr_buf(context, CL_MEM_USE_HOST_PTR, n * m * sizeof(T), arr, &err);
	::cl::Buffer param_buf(context, CL_MEM_USE_HOST_PTR, 2 * sizeof(T), params, &err);

	err = kernel.setArg(0, arr_buf);
	err = kernel.setArg(1, param_buf);

	::cl::CommandQueue q(context);
	err = q.enqueueNDRangeKernel(kernel, ::cl::NullRange, ::cl::NDRange(n, m));
	err = q.enqueueReadBuffer(arr_buf, CL_TRUE, 0, n * m * sizeof(T), arr);
}

using __t = float;
template void foo<__t>(__t(*)[], size_t, size_t, __t, __t);



_KSN_CL_END





int cl_init_result()
{
	return cl::detail::__construct_result;
}

int cl_pick_platform(size_t n)
{
	if (cl::detail::_cl_platform == n) return 0;

	if (n >= cl::detail::_opencl_impl_context.size()) return 1;

	cl::detail::_func_kernels.clear();
	cl::detail::_cl_platform = n;
	return 0;
}

int cl_pick_devices_type(uint64_t devs)
{
	if (devs == cl::detail::_cl_devices_mask) return cl::detail::__construct_result;

	cl::detail::_cl_devices_mask = devs;
	return cl::detail::__construct_result = cl::detail::__constructor(devs);
}

int cl_pick_platform_n_devices_types(size_t n, uint64_t devs)
{
	int code = cl_pick_platform(n);
	if (code != 0) return code;

	return cl_pick_devices_type(devs);
}

template<arithmetic T>
int cl_precompile(const char* f)
{
	auto *kernel = cl::detail::get_kernel<T>(f);
	return !(kernel);
}

int cl_create_kernel(void* pkernel, const char* src, const char* kernel_name)
{
	if (pkernel == nullptr || src == nullptr || kernel_name == nullptr) return 1;
	
	cl_int err;
	::cl::Program::Sources srcs(1, src);
	::cl::Program prog(*cl::detail::_context, srcs, &err);

	if (err != CL_SUCCESS) return err;
	err = prog.build("-cl-std=CL1.2");
	if (err != CL_SUCCESS)
	{
		if constexpr (!_KSN_HAS_EXCEPTIONS) return err;

		auto v = prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>();
		std::vector<std::string> excp;
		excp.reserve(v.size());
		for (auto&& [dev, str] : v)
		{
			excp.emplace_back(std::move(str));
		}
		_KSN_RAISE(excp);
	}

	*static_cast<::cl::Kernel*>(pkernel) = ::cl::Kernel(prog, kernel_name, &err);
	return err;
}





void f()
{
	::cl::Kernel k;
	
}


_KSN_END


#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif
