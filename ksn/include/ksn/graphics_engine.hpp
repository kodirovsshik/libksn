
#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>



static_assert(sizeof(uint8_t) == 1, "Strict integer sizes compliance required");
static_assert(sizeof(uint16_t) == 2, "Strict integer sizes compliance required");
static_assert(sizeof(uint32_t) == 4, "Strict integer sizes compliance required"); //-V112
static_assert(sizeof(uint64_t) == 8, "Strict integer sizes compliance required");

static_assert(sizeof(int8_t) == 1, "Strict integer sizes compliance required");
static_assert(sizeof(int16_t) == 2, "Strict integer sizes compliance required");
static_assert(sizeof(int32_t) == 4, "Strict integer sizes compliance required"); //-V112
static_assert(sizeof(int64_t) == 8, "Strict integer sizes compliance required");





#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4530) //Exceptions
#endif





_KSN_BEGIN

_KSN_GRAPHICS_BEGIN



using error_t = int;
struct error
{
	static constexpr error_t ok = 0;
	static constexpr error_t invalid_argument = 1;
	static constexpr error_t out_of_memory = 2;
	static constexpr error_t cl_no_such_platform = 3;
	static constexpr error_t cl_no_such_devices = 4;
};

union color_t
{
	struct
	{
		uint8_t b, g, r, a;
	};
	uint32_t color;

	constexpr color_t() noexcept
		: color(0xFF000000)
	{
	}
	constexpr color_t(uint8_t r, uint8_t g, uint8_t b) noexcept
		: r(r), g(g), b(b), a(0xFF)
	{
	}
	constexpr color_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a)  noexcept
		: r(r), g(g), b(b), a(a)
	{
	}
	constexpr color_t(uint32_t hex) noexcept
		: color(hex | 0xFF000000)
	{
	}
	constexpr color_t(uint32_t value, int unused) noexcept
		: color(value)
	{
	}

	constexpr color_t& operator=(const color_t&) noexcept = default;
	constexpr color_t& operator=(color_t&&) noexcept = default;
	constexpr color_t& operator=(uint32_t hex) noexcept
	{
		this->color = hex | 0xFF000000;
		return *this;
	}

};

struct vertex2_t
{
	float x, y;
};
struct vertex3_t
{
	float x, y, z;
};

struct surface_vectorized_t
{
	vertex3_t v[3];
};

struct surface_indexed_t
{
	uint32_t ndx[3];
};

struct texture_t
{
	//TODO: implement something
	color_t* data;
	uint16_t w, h;
};



class shape_buffer_t
{
	struct _shape_buffer_impl;
	ksn::fast_pimpl<_shape_buffer_impl, 4*4*sizeof(void*) + sizeof(void*), sizeof(void*), true, true, true, true> m_impl;

public:

	shape_buffer_t() noexcept;
	shape_buffer_t(const shape_buffer_t&) noexcept;
	shape_buffer_t(shape_buffer_t&&) noexcept;
	~shape_buffer_t() noexcept;

	
	uint32_t registrate(const vertex2_t*, size_t) noexcept;
	uint32_t registrate(const vertex3_t*, size_t) noexcept;
	uint32_t registrate(const surface_vectorized_t*, size_t) noexcept;
	uint32_t registrate(const surface_indexed_t*, size_t, size_t vertex_index_offset) noexcept;
	uint32_t registrate(const texture_t*, size_t) noexcept;

	bool reserve_surfaces(size_t) noexcept;
	bool reserve_textures(size_t) noexcept;
	bool reserve_texture_data(size_t pixels) noexcept;
	bool reserve_vertexes(size_t) noexcept;

	bool reserve_surfaces_add(size_t) noexcept;
	bool reserve_textures_add(size_t) noexcept;
	bool reserve_texture_data_add(size_t pixels) noexcept;
	bool reserve_vertexes_add(size_t) noexcept;

	//Sends all accumulated data and state to a videocard and preprocesses it
	int flush(bool reset = false) noexcept;
	//Clears local buffers state
	void reset() noexcept;
	//Clears all the state and deallocates the memory
	//i.e. bring to default-constructed state
	void free() noexcept;

	void invalidate_buffers() noexcept;

};



class graphics_engine_t
{
private:

	struct _graphics_engine_impl;
	fast_pimpl<_graphics_engine_impl, 1, 1, true, true, true, true> m_impl;


public:
	graphics_engine_t() noexcept;
	graphics_engine_t(const graphics_engine_t&) = delete;
	graphics_engine_t(graphics_engine_t&&) noexcept;
	~graphics_engine_t() noexcept;

	graphics_engine_t& operator=(const graphics_engine_t&) noexcept = delete;
	graphics_engine_t& operator=(graphics_engine_t&&) noexcept;
};






//b -> rb -> r -> rg -> g -> gb -> ...

_KSN_GRAPHICS_END

_KSN_END



#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif
