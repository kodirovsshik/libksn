#include <ksn/opencl_stuff.hpp>


_KSN_BEGIN

_KSN_CL_BEGIN

_KSN_DETAIL_BEGIN


const char* _iota_src = R"(

__kernel void iota(__global %1$s* ptr, __global %1$s* params)
{
	int id = get_global_size(0) * get_global_id(0) + get_global_id(1);

	ptr[id] = sqrt(ptr[id]);
}

)";


_KSN_DETAIL_END

_KSN_CL_END

_KSN_END
