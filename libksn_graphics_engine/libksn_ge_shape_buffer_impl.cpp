
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

//

#include <numeric>
#include <vector>
#include <random>



_KSN_BEGIN


namespace
{
	std::vector<cl_platform_id> _ge_cl_platforms;
	std::vector<std::vector<cl_device_id>> _ge_cl_devices;
	cl_context _ge_cl_context = nullptr;
	int _ge_cl_init_result;
	size_t _ge_cl_platform_index = 0;
	uint64_t _ge_cl_device_bitset = CL_DEVICE_TYPE_ALL;

	void ge_cl_notifier(const char* error_info, const void* private_info, size_t private_size, void* p)
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
				if (wrote != private_size)
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

	void _ge_cl_load_system_info()
	{
		_ge_cl_platforms.clear();
		_ge_cl_devices.clear();

		uint32_t num_platforms;
		clGetPlatformIDs(0, nullptr, &num_platforms);
		_ge_cl_platforms.resize(num_platforms);
		_ge_cl_devices.resize(num_platforms);
		clGetPlatformIDs(num_platforms, _ge_cl_platforms.data(), &num_platforms);

		for (uint32_t i = 0; i < num_platforms; ++i)
		{
			uint32_t num_devices;
			clGetDeviceIDs(_ge_cl_platforms[i], _ge_cl_device_bitset, 0, nullptr, &num_devices);
			_ge_cl_devices[i].resize(num_devices);
			clGetDeviceIDs(_ge_cl_platforms[i], _ge_cl_device_bitset, num_devices, _ge_cl_devices[i].data(), &num_devices);
		}
	};

}

int ge_cl_get_init_result() noexcept
{
	return _ge_cl_init_result;
}

int ge_cl_initialize() noexcept
{
	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)_ge_cl_platforms[_ge_cl_platform_index], 0};
	const auto& devices = _ge_cl_devices[_ge_cl_platform_index];

	_ge_cl_context = clCreateContext(properties, (uint32_t)devices.size(), devices.data(), ge_cl_notifier, nullptr, &_ge_cl_init_result);
	return _ge_cl_init_result;
}

void ge_cl_deinitialize() noexcept
{
	if (_ge_cl_context) clReleaseContext(_ge_cl_context);
	_ge_cl_context = nullptr;
}

int ge_cl_reinitialize() noexcept
{
	ge_cl_deinitialize();
	return ge_cl_initialize();
}

int ge_cl_pick_devices(uint64_t mask) noexcept
{
	if (_ge_cl_device_bitset != mask)
	{
		clReleaseContext(_ge_cl_context);
		_ge_cl_device_bitset = mask;
		_ge_cl_load_system_info();
		return ge_cl_reinitialize();
	}
	return _ge_cl_init_result;
}

int ge_cl_pick_platform(size_t n) noexcept
{
	if (n >= _ge_cl_platforms.size()) return 1;
	if (_ge_cl_platform_index != n)
	{
		clReleaseContext(_ge_cl_context);
		_ge_cl_platform_index = n;
		return ge_cl_reinitialize();
	}
	return _ge_cl_init_result;
}

int ge_cl_pick(size_t platform, uint64_t mask) noexcept
{
	if (_ge_cl_platform_index != platform || _ge_cl_device_bitset != mask)
	{
		clReleaseContext(_ge_cl_context);
		_ge_cl_platform_index = platform;
		_ge_cl_device_bitset = mask;
		if (_ge_cl_device_bitset != mask) _ge_cl_load_system_info();
		return ge_cl_reinitialize();
	}
	return _ge_cl_init_result;
}




namespace
{
	struct __lib_constructor_t
	{
		__lib_constructor_t()
		{
			_ge_cl_load_system_info();
			ge_cl_initialize();
		}
		~__lib_constructor_t()
		{
			ge_cl_deinitialize();
		}
	} static __lib_constructor;
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

	_shape_buffer_impl() noexcept
	{
		memset(this, 0, sizeof(*this));
	}
	~_shape_buffer_impl() noexcept
	{
		::free(this->m_surface_buffer);
		::free(this->m_texture_data_buffer);
		::free(this->m_texture_offset_buffer);
		::free(this->m_vertex_buffer);
	}


	surface_data* m_surface_buffer;
	size_t m_surface_capacity;
	size_t m_surface_count;
	cl_mem m_surface_cl_buffer;

	vertex3_t* m_vertex_buffer;
	size_t m_vertex_capacity;
	size_t m_vertex_count;
	cl_mem m_vertex_cl_buffer;

	uint32_t* m_texture_offset_buffer;
	uint32_t* m_texture_size_buffer;
	size_t m_texture_capacity;
	size_t m_texture_count;
	cl_mem m_texture_cl_buffer;

	color_t* m_texture_data_buffer;
	size_t m_texture_data_capacity;
	size_t m_texture_data_count;
	cl_mem m_texture_data_cl_buffer;

	template<typename T>
	bool reserve(T*& p, size_t& cap, size_t sz, size_t ncap)
	{
		if (ncap <= cap) return true;

		size_t diff = (ncap - cap) * sizeof(T);
		//No more than ~8 MB memory over what we actually have been requested for
		if constexpr (std::is_same_v<T, surface_data>)
		{ //as a first approximation, sizeof(surface_data) = 64
			ncap += (diff > 1024 * 1024 / 8 ? 1024 * 1024 / 8 : diff) / sizeof(T);
		}
		else
		{
			ncap += (diff > 1024 * 1024 * 8 ? 1024 * 1024 * 8 : diff) / sizeof(T);
		}
		T* new_p = (T*)malloc(sizeof(T) * ncap);
		if (new_p == nullptr)
		{
			new_p = (T*)malloc(sizeof(T) * ncap);
			if (new_p == nullptr) return false;
		}
		memcpy(new_p, p, sz * sizeof(T));

		::free(p);
		p = new_p;
		cap = ncap;
		return true;
	}





#define validate_return(nonzero, return_value) if (!(nonzero)) return return_value; ((void)0)

	uint32_t registrate(const surface_vectorized_t* p, size_t sz) noexcept
	{
		if (sz == 0) return uint32_t(-1);

		uint32_t result = (uint32_t)this->m_surface_count;

		validate_return(this->reserve_surfaces(this->m_surface_count + sz), -1);
		validate_return(this->reserve_vertexes(this->m_vertex_count + sz * 3), -1);

		vertex3_t* vb = this->m_vertex_buffer + this->m_vertex_count;
		while (sz--)
		{
			vb[0] = p->v[0];
			vb[1] = p->v[1];
			vb[2] = p->v[2];

			surface_data* ps = &this->m_surface_buffer[this->m_surface_count++];
			ps->vertexes[0] = uint32_t(this->m_vertex_count + 0);
			ps->vertexes[1] = uint32_t(this->m_vertex_count + 1);
			ps->vertexes[2] = uint32_t(this->m_vertex_count + 2);

			this->m_vertex_count += 3;
			vb += 3;
		}

		return result;
	}
	uint32_t registrate(const vertex2_t* vertex_ptr, size_t vertex_counter) noexcept
	{
		return this->registrate_vertexes<vertex2_t>(vertex_ptr, vertex_counter);
	}
	uint32_t registrate(const vertex3_t* vertex_ptr, size_t vertex_counter) noexcept
	{
		return this->registrate_vertexes<vertex3_t>(vertex_ptr, vertex_counter);
	}
	uint32_t registrate(const surface_indexed_t* surfaces_ptr, size_t surfaces_count, size_t vertex_offset) noexcept
	{
		if (surfaces_count == 0) return -1;

		validate_return(this->reserve_surfaces(this->m_surface_count + surfaces_count), -1);

		for (size_t i = 0; i < surfaces_count; ++i)
		{
			surface_data* p = &this->m_surface_buffer[i + this->m_surface_count];
			p->vertexes[0] = uint32_t(surfaces_ptr[i].ndx[0] + vertex_offset);
			p->vertexes[1] = uint32_t(surfaces_ptr[i].ndx[1] + vertex_offset);
			p->vertexes[2] = uint32_t(surfaces_ptr[i].ndx[2] + vertex_offset);
		}

		return (uint32_t)((this->m_surface_count += surfaces_count) - surfaces_count);
	}
	uint32_t registrate(const texture_t* p, size_t sz) noexcept
	{
		validate_return(sz != 0, -1);
		validate_return(this->reserve_textures(this->m_texture_count + sz), -1);

		size_t total_pixels = std::accumulate(p, p + sz, (size_t)0, [](size_t last, const texture_t& p) { return last + size_t(p.w) * p.h; });

		validate_return(this->reserve_texture_data(this->m_texture_data_count + total_pixels), -1);

		for (size_t i = 0; i < sz; ++i)
		{
			size_t current_size = size_t(p->w) * p->h;

			memcpy(this->m_texture_data_buffer + this->m_texture_data_count, p->data, current_size * sizeof(color_t));
			this->m_texture_offset_buffer[i + this->m_texture_count] = (uint32_t)this->m_texture_data_count;
			this->m_texture_data_count += current_size;

			this->m_texture_size_buffer[i + this->m_texture_count] = p->w | uint32_t(p->h << 16); //Place w in low word, h in high
		}
		return uint32_t((this->m_texture_count += sz) - sz);
	}

	template<class vertex_t>
	uint32_t registrate_vertexes(const vertex_t* p, size_t size)
	{
		validate_return(this->reserve_vertexes(this->m_vertex_count + size), -1);

		uint32_t result = (uint32_t)this->m_vertex_count;
		if constexpr (std::is_same_v<vertex_t, vertex3_t>)
		{
			memcpy(this->m_vertex_buffer + this->m_vertex_count, p, sizeof(vertex3_t) * size);
		}
		else
		{
			for (size_t i = 0; i < size; ++i)
			{
				this->m_vertex_buffer[i + this->m_vertex_count] = { p[i].x, p[i].y, 0 };
			}
			this->m_vertex_count += size;
		}

		return result;
	}

	int flush(bool b) noexcept
	{

		if (b) this->reset();
		return -1;
	}
	void reset() noexcept
	{
		this->m_surface_count = 0;
		this->m_texture_count = 0;
		this->m_texture_data_count = 0;
		this->m_vertex_count = 0;
	}
	void free() noexcept
	{
		::free(this->m_surface_buffer);
		::free(this->m_texture_data_buffer);
		::free(this->m_texture_offset_buffer);
		::free(this->m_vertex_buffer);

		memset(this, 0, sizeof(*this));
	}

	bool reserve_surfaces(size_t n) noexcept
	{
		return this->reserve(this->m_surface_buffer, this->m_surface_capacity, this->m_surface_count, n);
	}
	bool reserve_textures(size_t n) noexcept
	{
		if (n <= this->m_texture_capacity) return true;

		uint8_t* mem = (uint8_t*)malloc(8 * n);
		if (!mem) return false;
		uint32_t* p1 = (uint32_t*)mem;
		uint32_t* p2 = (uint32_t*)(mem + 4 * n);

		memcpy(p1, this->m_texture_offset_buffer, 4 * this->m_texture_count);
		memcpy(p2, this->m_texture_size_buffer, 4 * this->m_texture_count);

		::free(this->m_texture_offset_buffer);
		::free(this->m_texture_size_buffer);

		this->m_texture_offset_buffer = p1;
		this->m_texture_size_buffer = p2;

		return true;
	}
	bool reserve_texture_data(size_t n) noexcept
	{
		return this->reserve(this->m_texture_data_buffer, this->m_texture_data_capacity, this->m_texture_data_count, n);
	}
	bool reserve_vertexes(size_t n)
	{
		return this->reserve(this->m_vertex_buffer, this->m_vertex_capacity, this->m_vertex_count, n);
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
