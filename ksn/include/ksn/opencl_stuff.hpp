
#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>

//#include <concepts>





_KSN_BEGIN



//Returns last initialization result
int cl_init_result();

//If new platform specified, clears all precompiled functions and switches the platform index
int cl_pick_platform(size_t n);

//Switches devices types
int cl_pick_devices_type(uint64_t mask);

//See ksn::cl::cl_pick_platform and ksn::cl::cl_pick_devices_type
int cl_pick_platform_n_devices_types(size_t platform_index, uint64_t devices_types);

template<arithmetic T>
int cl_precompile(const char* algo_name);


int cl_create_kernel(void *pkernel, const char* src, const char* kernel_name);





_KSN_CL_BEGIN



template<arithmetic T>
void foo(T (*)[], size_t n, size_t m, T a, T b);

template void foo<float>(float(*)[], size_t, size_t, float, float);



_KSN_CL_END


_KSN_END
