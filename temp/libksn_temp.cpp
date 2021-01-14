
#include <stdio.h>

#include <ksn/opencl_stuff.hpp>
#include <ksn/stuff.hpp>

#pragma comment(lib, "libksn_opencl_stuff.lib")
#pragma comment(lib, "libksn_stuff.lib")
//#pragma comment(lib, "libksn_x86_instruction_set.lib")

#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "runtimeobject.lib")

#include <numeric>
#include <random>

#define CL_HPP_ENABLE_EXCEPTIONS 1
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/OpenCL.hpp>



const char* const src = R"(
__kernel void foo(__global int* ptr)
{
	
}
)";

int _main()
{
	cl_int err;
	cl::Kernel krnl;
	err = ksn::cl_create_kernel(&krnl, src, "foo");
	
	int x = 42;
	cl::Buffer buff(krnl.getInfo<CL_KERNEL_CONTEXT>(), CL_MEM_USE_HOST_PTR, sizeof(x), &x, &err);
	err = krnl.setArg(0, buff);

	cl::CommandQueue q;
	err = q.enqueueTask(krnl);
	err = q.enqueueReadBuffer(buff, CL_TRUE, 0, sizeof(x), &x);

	return 0;
}


int main()
{
	try
	{
		return _main();
	}
	catch (std::vector<std::string> v)
	{
		for (const auto& s : v)
		{
			printf("%s\n", s.c_str());
		}
	}
	catch (cl::Error e)
	{
		printf("OpenCL error %i: %s\n", e.err(), e.what());
	}
	catch (std::exception e)
	{
		printf("STD exception %s\n", e.what());
	}
	catch (...)
	{
		printf("Unknown error");
	}
	
	
	
}
