
#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>


#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4530) //Exceptions
#endif


_KSN_BEGIN


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
	ksn::fast_pimpl<_shape_buffer_impl, 136, sizeof(void*), true, true, true, true> m_impl;

public:

	shape_buffer_t() noexcept;
	shape_buffer_t(const shape_buffer_t&) noexcept;
	shape_buffer_t(shape_buffer_t&&) noexcept;
	~shape_buffer_t() noexcept;

	
	uint32_t registrate(const vertex2_t*, size_t) noexcept;
	uint32_t registrate(const vertex3_t*, size_t) noexcept;
	uint32_t registrate(const surface_vectorized_t*, size_t) noexcept;
	uint32_t registrate(const surface_indexed_t*, size_t, size_t index_registred_offset) noexcept;
	uint32_t registrate(const texture_t*, size_t) noexcept;

	bool reserve_surfaces(size_t) noexcept;
	bool reserve_textures(size_t) noexcept;
	bool reserve_texture_data(size_t pixels) noexcept;
	bool reserve_vertexes(size_t) noexcept;

	//Sends all accumulated data and state to a videocard and preprocesses it
	int flush(bool reset = false) noexcept;
	//Clears its state
	void reset() noexcept;
	//Clears its state and deallocates the memory
	void free() noexcept;

	//void reset_vertexes() noexcept;
	
	//void replace(size_t, surface_t&) noexcept;

};



//b -> rb -> r -> rg -> g -> gb -> ...

_KSN_END



#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif
