
#ifndef _LIBKSN_GRAPHICS_SRC_SURFACE_PREPROCESS_IXX_
#define _LIBKSN_GRAPHICS_SRC_SURFACE_PREPROCESS_IXX_







constexpr static const char* ge_src_surface_proprocess = R"(

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef struct 
{
	float3 b_a; float3 c_a;
	uint32_t vertexes[3];
	uint32_t texture_index;
	//Some constants for barycentric coordinates
	float bary_dbb, bary_dbc, bary_dcc, bary_inv_denom;
} surface_data;

__kernel void surface_preprocess(__global surface_data* pb_s, __global float3* pb_v)
{
	uint32_t i = get_global_id(0) * get_local_size(0) + get_local_id(0);
	printf((const char __constant*) "\r[OpenCL][surface_preprocess]                      \nKernel called\ni = %u\nglobal_size = %u\nlocal_size = %u\nglobal_id = %u\nlocal_id = %i\n\n",
		i, (uint32_t)get_global_size(0), (uint32_t)get_local_size(0), (uint32_t)get_global_id(0), (uint32_t)get_local_id(0));
	
	__global surface_data* p_s = pb_s + i;
	__global float3* p_v = pb_v + i;
	p_s->b_a = p_v[p_s->vertexes[1]] - p_v[p_s->vertexes[0]];
	p_s->c_a = p_v[p_s->vertexes[2]] - p_v[p_s->vertexes[0]];

#define b p_s->b_a
#define c p_s->c_a

	p_s->bary_dbb = b.x * b.x + b.y * b.y + b.z * b.z;
	p_s->bary_dbc = b.x * c.x + b.y * c.y + b.z * c.z;
	p_s->bary_dcc = c.x * c.x + c.y * c.y + c.z * c.z;
	p_s->bary_inv_denom = 1.0f / (p_s->bary_dbb * p_s->bary_dcc - p_s->bary_dbc * p_s->bary_dbc);

#undef b
#undef c
}

)";

#endif //!_LIBKSN_GRAPHICS_SRC_SIRFACE_PREPROCESS_IXX_
