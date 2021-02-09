
#include <ksn/ksn.hpp>

#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 4530) //Exceptions
#pragma warning(disable : 4996) // >:cc
#endif

#include <ksn/graphics_engine.hpp>
#include <ksn/stuff.hpp>

#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110

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



_KSN_BEGIN

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



struct shape_buffer_t::_shape_buffer_impl
{
	struct surface_data
	{
		uint32_t vertexes[3];
		uint32_t texture_index = -1;
		float normal[3];
		//Some constants for barycentric coordinates
		float bary_d00, bary_d01, bary_d11, bary_inv_denom;
		float b_a[3]; float c_a[3];
	};
	static_assert(sizeof(surface_data) == 68);
	static_assert(alignof(surface_data) == 4);

	template<typename T>
	struct cl_buffer_adapter_t
	{
		T* m_data;
		size_t m_count, m_capacity;
		cl_mem m_cl;

		bool reserve(size_t new_capacity) noexcept
		{
			if (new_capacity <= this->m_capacity) return true;

			size_t memsize = new_capacity * sizeof(T);
			T* new_ptr = (T*)malloc(memsize);
			if (new_ptr == nullptr) return false;
			
			memcpy(new_ptr, this->m_data, this->m_count * sizeof(T));
			this->m_data = p;
			this->m_capacity = new_capacity;
			return true;
		}
	};

	_shape_buffer_impl() noexcept
	{
		memset(this, 0, sizeof(*this));
		this->m_cl_device_types = CL_DEVICE_TYPE_ALL;
	}
	~_shape_buffer_impl() noexcept
	{
	}

	uint64_t m_cl_device_types;
	cl_context m_cl_context;
	cl_buffer_adapter_t<surface_data> m_surface_buffer;
	cl_buffer_adapter_t<vertex3_t> m_vertex_buffer;
	cl_buffer_adapter_t<uint64_t> m_texture_descriptor_buffer;
	cl_buffer_adapter_t<color_t> m_texture_data_buffer;
	uint32_t m_cl_platform_number;

	
#define validate_return(nonzero, return_value) if (!(nonzero)) return return_value; ((void)0)


	uint32_t registrate(const surface_vectorized_t* p, size_t sz) noexcept
	{
	}
	uint32_t registrate(const vertex2_t* vertex_ptr, size_t vertex_counter) noexcept
	{
	}
	uint32_t registrate(const vertex3_t* vertex_ptr, size_t vertex_counter) noexcept
	{
	}
	uint32_t registrate(const surface_indexed_t* surfaces_ptr, size_t surfaces_count, size_t vertex_offset) noexcept
	{
	}
	uint32_t registrate(const texture_t* p, size_t sz) noexcept
	{
		validate_return(sz != 0, -1);
		//validate_return(this->reserve_textures(this->m_texture_count + sz), -1);

		size_t total_pixels = std::accumulate(p, p + sz, (size_t)0, [](size_t last, const texture_t& p) { return last + size_t(p.w) * p.h; });

		//validate_return(this->reserve_texture_data(this->m_texture_data_count + total_pixels), -1);
	}

	template<class vertex_t>
	uint32_t registrate_vertexes(const vertex_t* p, size_t size)
	{
		//if constexpr (std::is_same_v<vertex_t, vertex3_t>)
	}

	int flush(bool b) noexcept
	{
		int temp_result;
		static constexpr cl_mem_flags flags = CL_MEM_READ_ONLY;

		if (b) this->reset();
		return 0;
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

uint32_t shape_buffer_t::registrate(const vertex2_t* p, size_t sz) noexcept
{
	return this->m_impl->registrate(p, sz);
}
uint32_t shape_buffer_t::registrate(const vertex3_t* p, size_t sz) noexcept
{
	return this->m_impl->registrate(p, sz);
}
uint32_t shape_buffer_t::registrate(const surface_vectorized_t* p, size_t sz) noexcept
{
	return this->m_impl->registrate(p, sz);
}
uint32_t shape_buffer_t::registrate(const surface_indexed_t* p, size_t sz, size_t off) noexcept
{
	return this->m_impl->registrate(p, sz, off);
}
uint32_t shape_buffer_t::registrate(const texture_t* p, size_t sz) noexcept
{
	return this->m_impl->registrate(p, sz);
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





_KSN_END
