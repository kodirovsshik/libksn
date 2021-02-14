
#include <ksn/ksn.hpp>

#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 4530) //Exceptions
#pragma warning(disable : 4996) // >:cc
#pragma warning(disable : 26439) // >:cc
#endif

#include <ksn/graphics.hpp>
#include <ksn/stuff.hpp>

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/opencl.hpp>



//struct _cl_mem {};
//struct _cl_context {};
//struct _cl_platform_id {};
//struct _cl_device_id {};
//
//typedef _cl_mem* cl_mem;
//typedef _cl_context* cl_context;
//typedef _cl_platform_id* cl_platform_id;
//typedef _cl_device_id* cl_device_id;
//
//typedef uint64_t cl_bitset;
//typedef uint64_t cl_context_properties;
//typedef int32_t cl_int;
//typedef uint32_t cl_uint;
//
//#define CL_DEVICE_TYPE_ALL uint64_t(-1)
//#define CL_CONTEXT_PLATFORM -1
//
//cl_context clCreateContext(cl_context_properties*, cl_uint devices_number, const cl_device_id*,
//	void (*p_callback)(const char*, const void*, size_t, void* user_data)
//	, void* callback_data, cl_int* err)
//{
//	*err = 0;
//	char data[] = "konpeko konpeko konpeko, hororaifu san-kisei no usada pekora peko doumo, doumo, doumo";
//	p_callback("Not imlemented", data, sizeof(data), callback_data);
//	return nullptr;
//}
//cl_int clReleaseContext(cl_context)
//{
//	return 0;
//}
//cl_int clReleaseBuffer(cl_mem)
//{
//	return 0;
//}
//cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id* p, cl_uint* actual_num)
//{
//	if (actual_num) *actual_num = 1;
//	if (num_entries == 0) return 0;
//	if (p) *p = (cl_platform_id)1;
//	return 0;
//}
//cl_int clGetDeviceIDs(cl_platform_id, cl_bitset device_types, cl_uint num_entries, cl_device_id* p, cl_uint* actual_num)
//{
//	if (actual_num) *actual_num = 1;
//	if (num_entries == 0) return 0;
//	if (p) *p = (cl_device_id)2;
//	return 0;
//}





#include <numeric>
#include <vector>
#include <random>





#define validate_return(nonzero, return_value) if (!(nonzero)) return return_value; ((void)0)
#define validate_zero(errcode) { auto __ = errcode; if (__) return __; } ((void)0)
#define validate_value(errcode, success) { auto __ = errcode; if (__ != success) return __; } ((void)0)





_KSN_BEGIN


_KSN_GRAPHICS_BEGIN

static void ge_cl_notifier(const char* error_info, const void* private_info, size_t private_size, void* p)
{
	fprintf(stderr, "\aOpenCL context error callback invoked\n%s\n", error_info);

	std::mt19937_64 engine;
	engine.seed(time(nullptr));
		
	static constexpr size_t max_tries = 128;

	auto binary_writer = [&]
	(FILE* fd) -> size_t
	{
		return fwrite(private_info, 1, private_size, fd);
	};

	auto text_writer = [&]
	(FILE* fd) -> size_t
	{
		return ksn::memory_dump(private_info, private_size, 16, 0, fd);
	};

	auto try_write = [&]
	(bool binary) -> bool
	{
		size_t tries = max_tries;
		char buffer[_MAX_PATH];
		FILE* fd;

		while (tries--)
		{
			unsigned long long val = engine();
			sprintf_s(buffer, _MAX_PATH, "dump%llu.%s", val, binary ? "bin" : "txt");
			if ((fd = fopen(buffer, binary ? "w" : "wb")) == nullptr) continue;

			size_t wrote = binary ? binary_writer(fd) : text_writer(fd);
			fclose(fd);
			if (wrote < private_size)
			{
				remove(buffer);
				continue;
			}
			fprintf(stderr, "OpenCL data %s dump saved to %s\n", binary ? "binary" : "text", buffer);
			return true;
		}

		fprintf(stderr, "OpenCL data %s dump save failure\n", binary ? "binary" : "text");
		return false;
	};


	int ok = 0;
	if (try_write(false)) ok |= 1;
	if (try_write(true)) ok |= 2;
	if (ok == 0)
	{
		fwrite("OpenCL data dump:\n", 1, 18, stderr);
		ksn::memory_dump(private_info, private_size, 16, 0, stderr);
		fputc('\n', stderr);
	}
}


static constexpr uint32_t ge_cl_platforms_static_count = 64;
static cl_platform_id ge_cl_platforms_static[ge_cl_platforms_static_count];
static cl_platform_id* ge_cl_platforms;


#include "libksn_graphics_src_surface_preprocess.cpp"


static constexpr const char* ge_cl_programs[] =
{
	ge_src_surface_proprocess 
};

static constexpr size_t ge_cl_programs_lengths[sizeof(ge_cl_programs) / sizeof(*ge_cl_programs)] =
{
	std::char_traits<char>::length(ge_cl_programs[0])
};


namespace
{
	error_t ge_cl_platforms_load() noexcept
	{
		error_t result = error::ok;

		uint32_t n;
		clGetPlatformIDs(0, nullptr, &n);
		if (n > ge_cl_platforms_static_count)
		{
			ge_cl_platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * n);
			if (ge_cl_platforms == nullptr)
			{
				ge_cl_platforms = ge_cl_platforms_static;
				n = ge_cl_platforms_static_count;
				result = error::out_of_memory;
			}
		}
		else
		{
			ge_cl_platforms = ge_cl_platforms_static;
		}
		clGetPlatformIDs(n, ge_cl_platforms, &n);
		return result;
	}

	void ge_cl_platforms_unload() noexcept
	{
		if (ge_cl_platforms != ge_cl_platforms_static) free(ge_cl_platforms);
		ge_cl_platforms = nullptr;
	}

	struct __lib_constructor
	{
		__lib_constructor() noexcept
		{
			ge_cl_platforms_load();
		}
		~__lib_constructor() noexcept
		{
			ge_cl_platforms_unload();
		}
	} static constructor;
}

int ge_cl_reload_platforms() noexcept
{
	ge_cl_platforms_unload();
	return ge_cl_platforms_load();
}



struct shape_buffer_t::_shape_buffer_impl
{
	struct surface_data
	{
		cl_float3 b_a, c_a;
		cl_uint vertexes[3];
		cl_uint texture_index;
		//Some constants for barycentric coordinates
		float bary_d00, bary_d01, bary_d11, bary_inv_denom;
	};
	static_assert(sizeof(surface_data) == 64);
	static_assert(alignof(surface_data) == 4);

	template<typename T>
	struct cl_buffer_adapter_t
	{
		using type = T;

		T* m_data;
		size_t m_count, m_capacity;
		cl_mem m_cl;

		bool reserve(size_t new_capacity) noexcept
		{
			if (new_capacity <= this->m_capacity) return true;

			size_t memsize = new_capacity * sizeof(T);
			T* new_ptr = (T*)malloc(memsize);
			if (new_ptr == nullptr) return false;
			
			if (this->m_data)
			{
				memcpy(new_ptr, this->m_data, this->m_count * sizeof(T));
				::free(this->m_data);
			}

			this->m_data = new_ptr;
			this->m_capacity = new_capacity;
			return true;
		}

		bool reserve_more(size_t additional_units) noexcept
		{
			return this->reserve(this->m_count + additional_units);
		}

		int cl_create(cl_context context, cl_mem_flags flags) noexcept
		{
			if (this->m_cl)
			{
				size_t buffer_capacity = 0;
				validate_zero(clGetMemObjectInfo(this->m_cl, CL_MEM_SIZE, sizeof(size_t), &buffer_capacity, nullptr));
				if (this->m_count * sizeof(T) <= buffer_capacity) return 0; //Already as much as needed
			}

			int err;
			size_t buffer_size = this->m_count * sizeof(*this->m_data);
			this->m_cl = clCreateBuffer(context, flags, buffer_size + bool(!buffer_size), nullptr, &err);
			return err;
		}

		int cl_invalidate() noexcept
		{
			if (this->m_cl == nullptr) return 0;
			
			return clReleaseMemObject(this->m_cl);
		}

		void free()
		{
			::free(this->m_data);
			this->m_data = nullptr;
			this->m_capacity = 0;
			this->m_count = 0;
		}
	};

	_shape_buffer_impl(uint32_t cl_platform_number = 0, cl_bitfield cl_device_types = CL_DEVICE_TYPE_ALL, int* err = nullptr) noexcept
	{
		memset(this, 0, sizeof(*this));
		this->create_context(cl_platform_number, cl_device_types, err);
	}
	~_shape_buffer_impl() noexcept
	{
	}


	cl_buffer_adapter_t<surface_data> m_surface_buffer;
	cl_buffer_adapter_t<vertex4_t> m_vertex_buffer;
	cl_buffer_adapter_t<uint64_t> m_texture_descriptor_buffer;
	cl_buffer_adapter_t<color_t> m_texture_data_buffer;
	
	cl_context m_cl_context;
	cl_program m_program;



	void create_context(uint32_t platform, cl_bitfield devices, int* err) noexcept
	{
		cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)ge_cl_platforms[platform], 0};

		this->m_cl_context = clCreateContextFromType(props, devices, ge_cl_notifier, nullptr, err);
	}

	int create_context(uint32_t platform, cl_bitfield devices)
	{
		int result;
		this->create_context(platform, devices, &result);
		return result;
	}

	error_t registrate(const surface_vectorized_t* p, size_t sz, uint32_t& result) noexcept
	{
		auto& vertex_buffer = this->m_vertex_buffer;
		auto& surface_buffer = this->m_surface_buffer;
		
		validate_return(surface_buffer.reserve_more(sz), error::out_of_memory);
		validate_return(vertex_buffer.reserve_more(sz * 3), error::out_of_memory);

		result = (uint32_t)surface_buffer.m_count;

		memcpy(vertex_buffer.m_data + vertex_buffer.m_count, p, sizeof(vertex3_t) * sz * 3);

		for (const surface_vectorized_t* pe = p + sz; p != pe; ++p)
		{
			surface_data* p_surface = &surface_buffer.m_data[surface_buffer.m_count++];
			p_surface->vertexes[0] = uint32_t(vertex_buffer.m_count + 0);
			p_surface->vertexes[1] = uint32_t(vertex_buffer.m_count + 1);
			p_surface->vertexes[2] = uint32_t(vertex_buffer.m_count + 2);

			vertex_buffer.m_count += 3;
		}

		return error::ok;
	}
	error_t registrate(const vertex2_t* vertex_ptr, size_t vertex_counter, uint32_t& result) noexcept
	{
		return this->registrate_vertexes<vertex2_t>(vertex_ptr, vertex_counter, result);
	}
	error_t registrate(const vertex3_t* vertex_ptr, size_t vertex_counter, uint32_t& result) noexcept
	{
		return this->registrate_vertexes<vertex3_t>(vertex_ptr, vertex_counter, result);
	}
	error_t registrate(const surface_indexed_t* p, size_t sz, size_t off, uint32_t& result) noexcept
	{
		auto& surface_buffer = this->m_surface_buffer;

		validate_return(surface_buffer.reserve_more(sz), error::out_of_memory);
		
		result = (uint32_t)surface_buffer.m_count;
		
		for (const surface_indexed_t* pe = p + sz; p != pe; ++p)
		{
			surface_data* p_surface = &surface_buffer.m_data[surface_buffer.m_count++];
			p_surface->vertexes[0] = uint32_t(p->ndx[0] + off);
			p_surface->vertexes[1] = uint32_t(p->ndx[1] + off);
			p_surface->vertexes[2] = uint32_t(p->ndx[2] + off);
			p_surface->texture_index = -1;
		}

		return error::ok;
	}
	error_t registrate(const texture_t* p, size_t sz, uint32_t& result) noexcept
	{
		//validate_return(sz != 0, error::invalid_argument);
		auto& descriptor_buffer = this->m_texture_descriptor_buffer;
		auto& data_buffer = this->m_texture_data_buffer;
		
		validate_return(this->m_texture_descriptor_buffer.reserve_more(sz), error::out_of_memory);

		size_t total_pixels = std::accumulate(p, p + sz, (size_t)0, [](size_t last, const texture_t& p) { return last + size_t(p.w) * p.h; });
		validate_return(this->m_texture_data_buffer.reserve_more(total_pixels), error::out_of_memory);

		result = (uint32_t)descriptor_buffer.m_count;

		for (const texture_t* pe = p + sz; p != pe; ++p)
		{
			descriptor_buffer.m_data[descriptor_buffer.m_count++] = (uint32_t)data_buffer.m_count | ((uint64_t)p->w << 32) | ((uint64_t)p->h << 48);

			size_t size = size_t(p->w) * p->h;
			memcpy(data_buffer.m_data + data_buffer.m_count, p->data, size * sizeof(color_t));
			data_buffer.m_count += size;
		}

		return error::ok;
	}
	template<class vertex_t>
	error_t registrate_vertexes(const vertex_t* p, size_t sz, uint32_t& result)
	{
		static constexpr bool is_3 = std::is_same_v<vertex3_t, vertex_t>;

		auto& vertex_buffer = this->m_vertex_buffer;
		validate_return(this->m_vertex_buffer.reserve_more(sz), error::out_of_memory);

		result = (uint32_t)vertex_buffer.m_count;

		for (const vertex_t* pe = p + sz; p != pe; ++p)
		{
			if constexpr (is_3) 
				vertex_buffer.m_data[vertex_buffer.m_count++] = vertex4_t{ p->x, p->y, p->z };
			else
				vertex_buffer.m_data[vertex_buffer.m_count++] = vertex4_t{ p->x, p->y, 0 };
		}

		return error::ok;
	}

	error_t flush(bool _reset) noexcept
	{
		int temp_result;
		static constexpr cl_mem_flags flags = CL_MEM_READ_ONLY;
		ksn::malloc_guard mem;

		validate_zero(this->m_surface_buffer.cl_create(this->m_cl_context, flags));
		validate_zero(this->m_texture_data_buffer.cl_create(this->m_cl_context, flags));
		validate_zero(this->m_texture_descriptor_buffer.cl_create(this->m_cl_context, flags));
		validate_zero(this->m_vertex_buffer.cl_create(this->m_cl_context, flags));

		size_t devices_memsize = 0;
		clGetContextInfo(this->m_cl_context, CL_CONTEXT_DEVICES, 0, nullptr, &devices_memsize);

		cl_device_id* devices = (cl_device_id*)mem.alloc(devices_memsize);
		validate_return(devices, error::out_of_memory);

		temp_result = clGetContextInfo(this->m_cl_context, CL_CONTEXT_DEVICES, devices_memsize, devices, nullptr);
		validate_zero(temp_result);

		cl_command_queue q = clCreateCommandQueue(this->m_cl_context, devices[0], 0, &temp_result);
		validate_return(q, temp_result);

		auto _flush_buffer = [&]<typename elem_t>
		(const cl_buffer_adapter_t<elem_t>& buffer) -> int
		{
			int err = 0;
			if (buffer.m_data && buffer.m_count)
			{
				err = clEnqueueWriteBuffer(q, buffer.m_cl, CL_FALSE, 0, buffer.m_count * sizeof(*buffer.m_data), buffer.m_data, 0, nullptr, nullptr);
			}
			return err;
		};
		

#define flush_buffer(buffer) validate_zero(_flush_buffer(buffer))
		flush_buffer(this->m_surface_buffer);
		flush_buffer(this->m_texture_data_buffer);
		flush_buffer(this->m_texture_descriptor_buffer);
		flush_buffer(this->m_vertex_buffer);
#undef flush_buffer

		if (this->m_program == nullptr)
		{
			this->m_program = clCreateProgramWithSource(this->m_cl_context, sizeof(ge_cl_programs) / sizeof(*ge_cl_programs), (const char**)ge_cl_programs, ge_cl_programs_lengths, &temp_result);
			validate_zero(temp_result);
			temp_result = clBuildProgram(this->m_program, uint32_t(devices_memsize / sizeof(cl_device_id)), devices, "-cl-std=CL1.2", nullptr, nullptr);
			if (temp_result)
			{
				size_t build_log_size;
				clGetProgramBuildInfo(this->m_program, devices[0], CL_PROGRAM_BUILD_LOG, 0, nullptr, &build_log_size);
				build_log_size++;
				char* build_log = (char*)mem.alloc(build_log_size);
				if (build_log)
				{
					clGetProgramBuildInfo(this->m_program, devices[0], CL_PROGRAM_BUILD_LOG, build_log_size, build_log, nullptr);
					printf("\nBUILD LOG:\n%s\n\n", build_log);
				}
			}
			validate_zero(temp_result);
		}

		
		cl_kernel preprocess_flush_kernel = clCreateKernel(this->m_program, "surface_preprocess", &temp_result);
		validate_zero(temp_result);

		clSetKernelArg(preprocess_flush_kernel, 0, sizeof(cl_mem*), &this->m_surface_buffer.m_cl);
		clSetKernelArg(preprocess_flush_kernel, 1, sizeof(cl_mem*), &this->m_vertex_buffer.m_cl);

		size_t z1 = 0;

		clEnqueueNDRangeKernel(q, preprocess_flush_kernel, 1, &z1, &this->m_surface_buffer.m_count, nullptr, 0, nullptr, nullptr);
		//clEnqueueReadBuffer(q, this->m_surface_buffer.m_cl, CL_FALSE, 0, this->m_surface_buffer.m_count * sizeof(*this->m_surface_buffer.m_data), this->m_surface_buffer.m_data, 0, 0, 0);

		temp_result = clFinish(q);
		clReleaseKernel(preprocess_flush_kernel);

		validate_zero(temp_result);

		if (_reset) this->reset();
		return 0;
	}

	void reset() noexcept
	{
		this->m_surface_buffer.m_count = 0;
		this->m_vertex_buffer.m_count = 0;
		this->m_texture_data_buffer.m_count = 0;
		this->m_texture_descriptor_buffer.m_count = 0;
	}

	void free()
	{
		this->m_surface_buffer.free();
		this->m_texture_data_buffer.free();
		this->m_texture_descriptor_buffer.free();
		this->m_vertex_buffer.free();
		this->reset();
	}

	void invalidate_buffers() noexcept
	{
		this->m_surface_buffer.cl_invalidate();
		this->m_texture_data_buffer.cl_invalidate();
		this->m_texture_descriptor_buffer.cl_invalidate();
		this->m_vertex_buffer.cl_invalidate();
	}

	bool reserve_surfaces(size_t new_cap) noexcept
	{
		return this->m_surface_buffer.reserve(new_cap);
	}
	bool reserve_textures(size_t new_cap) noexcept
	{
		return this->m_texture_descriptor_buffer.reserve(new_cap);
	}
	bool reserve_texture_data(size_t new_cap) noexcept
	{
		return this->m_texture_data_buffer.reserve(new_cap);
	}
	bool reserve_vertexes(size_t new_cap) noexcept
	{
		return this->m_vertex_buffer.reserve(new_cap);
	}
};




shape_buffer_t::shape_buffer_t() noexcept
{
}
shape_buffer_t::shape_buffer_t(const shape_buffer_t& r) noexcept
	: m_impl(r.m_impl)
{
}
shape_buffer_t::shape_buffer_t(shape_buffer_t&& r) noexcept
	: m_impl(std::move(r.m_impl))
{
}
shape_buffer_t::~shape_buffer_t() noexcept
{
}

uint32_t shape_buffer_t::registrate(const vertex2_t* p, size_t sz, error_t* err) noexcept
{
	uint32_t result = -1;
	error_t err_local = this->m_impl->registrate(p, sz, result);
	if (err) *err = err_local;
	return result;
}
uint32_t shape_buffer_t::registrate(const vertex3_t* p, size_t sz, error_t* err) noexcept
{
	uint32_t result = -1;
	error_t err_local = this->m_impl->registrate(p, sz, result);
	if (err) *err = err_local;
	return result;
}
uint32_t shape_buffer_t::registrate(const surface_vectorized_t* p, size_t sz, error_t* err) noexcept
{
	uint32_t result = -1;
	error_t err_local = this->m_impl->registrate(p, sz, result);
	if (err) *err = err_local;
	return result;
}
uint32_t shape_buffer_t::registrate(const surface_indexed_t* p, size_t sz, size_t off, error_t* err) noexcept
{
	uint32_t result = -1;
	error_t err_local = this->m_impl->registrate(p, sz, off, result);
	if (err) *err = err_local;
	return result;
}
uint32_t shape_buffer_t::registrate(const texture_t* p, size_t sz, error_t* err) noexcept
{
	uint32_t result = -1;
	error_t err_local = this->m_impl->registrate(p, sz, result);
	if (err) *err = err_local;
	return result;
}

int shape_buffer_t::flush(bool release) noexcept
{
	return this->m_impl->flush(release);
}

bool shape_buffer_t::reserve_surfaces(size_t n) noexcept
{
	return this->m_impl->reserve_surfaces(n);
}
bool shape_buffer_t::reserve_vertexes(size_t n) noexcept
{
	return this->m_impl->reserve_vertexes(n);
}
bool shape_buffer_t::reserve_texture_data(size_t n) noexcept
{
	return this->m_impl->reserve_texture_data(n);
}
bool shape_buffer_t::reserve_textures(size_t n) noexcept
{
	return this->m_impl->reserve_textures(n);
}


void shape_buffer_t::invalidate_buffers() noexcept
{
	this->m_impl->invalidate_buffers();
}

bool shape_buffer_t::reserve_surfaces_add(size_t add) noexcept
{
	return this->m_impl->m_surface_buffer.reserve_more(add);
}
bool shape_buffer_t::reserve_textures_add(size_t add) noexcept
{
	return this->m_impl->m_texture_descriptor_buffer.reserve_more(add);
}
bool shape_buffer_t::reserve_texture_data_add(size_t add) noexcept
{
	return this->m_impl->m_texture_data_buffer.reserve_more(add);
}
bool shape_buffer_t::reserve_vertexes_add(size_t add) noexcept
{
	return this->m_impl->m_vertex_buffer.reserve_more(add);
}




_KSN_GRAPHICS_END

_KSN_END
