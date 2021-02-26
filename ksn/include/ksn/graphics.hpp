
#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/math_constexpr.hpp>



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
	constexpr color_t(const color_t&) noexcept = default;
	constexpr color_t(color_t&&) noexcept = default;

	constexpr color_t& operator=(const color_t&) noexcept = default;
	constexpr color_t& operator=(color_t&&) noexcept = default;
	constexpr color_t& operator=(uint32_t hex) noexcept
	{
		this->color = hex | 0xFF000000;
		return *this;
	}

	operator uint32_t() const noexcept
	{
		return this->color;
	}

	uint32_t rgba() const noexcept
	{
		return (this->color & 0xFF00FF00) | (this->r) | (this->b << 16);

		//uint32_t x = 0;
		//x |= this->r;
		//x <<= 8;
		//x |= this->g;
		//x <<= 8;
		//x |= this->b;
		//x <<= 8;
		//x |= this->a;
		//return x;
	}
	uint32_t bgra() const noexcept
	{
		return this->color;
	}
};

struct color_hsv_t
{
	int16_t hue : 10; //0 - 359
	uint8_t saturation : 7; //0-100
	uint8_t value : 7; //0-100
	uint8_t alpha; //0-255



	constexpr color_hsv_t() noexcept
	{
		sizeof(*this);
		this->hue = 0;
		this->saturation = 0;
		this->value = 0;
		this->alpha = 255;
	}
	constexpr color_hsv_t(color_t rgb) noexcept
	{
		uint8_t max_color = rgb.r;
		if (rgb.g > max_color) max_color = rgb.g;
		if (rgb.b > max_color) max_color = rgb.b;
		this->value = max_color * 100 / 255;

		max_color += max_color == 1;

		uint8_t max_diff = 0;
#define check_diff(a, b) { uint8_t temp = a >= b ? a - b : b - a; if (temp > max_diff) max_diff = temp; } ((void)0)
		check_diff(rgb.r, rgb.b);
		check_diff(rgb.r, rgb.g);
		check_diff(rgb.b, rgb.g);
#undef check_diff
		this->saturation = max_diff * 100 / max_color;


		uint16_t sum = rgb.r + rgb.b + rgb.g;
		sum += sum == 0;
		float hx = (rgb.r - (rgb.g + rgb.b) / 2.0f) / sum;
		float hy = (rgb.g - rgb.b) * ksn::sin(2 * KSN_PIf / 3) / sum;
		this->hue = int16_t(180 / KSN_PIf * ksn::fmod(ksn::atan2(hy, hx, 0.01f), 2 * KSN_PIf));

		this->alpha = rgb.a;
	}
	constexpr color_hsv_t(uint16_t hue, uint8_t saturation, uint8_t value) noexcept
		: hue(hue), saturation(saturation), value(value), alpha(255)
	{
	}
	constexpr color_hsv_t(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alhpa) noexcept
		: hue(hue), saturation(saturation), value(value), alpha(alhpa)
	{
	}

	constexpr color_hsv_t(const color_hsv_t&) noexcept = default;
	constexpr color_hsv_t(color_hsv_t&&) noexcept = default;

	constexpr color_t to_rgb() const noexcept
	{
		color_t result;
		
		float max_color = 2.5500001f * this->value;
		float hue_rad = this->hue * KSN_PIf / 180.0f;
		constexpr float pi3 = KSN_PIf / 3;

		result.r = uint8_t(max_color * (this->saturation / 100.f * (limited_cos(hue_rad +- 0 * pi3) - 1) + 1));
		result.g = uint8_t(max_color * (this->saturation / 100.f * (limited_cos(hue_rad - 2 * pi3) - 1) + 1));
		result.b = uint8_t(max_color * (this->saturation / 100.f * (limited_cos(hue_rad + 2 * pi3) - 1) + 1));
		result.a = this->alpha;
		return result;
	}
	constexpr operator color_t() const noexcept
	{
		return this->to_rgb();
	}


private:

	//cos(x)+0.5 but bounded by y º [0; 1]
	constexpr static float limited_cos(float hue)
	{
		constexpr float pi3 = KSN_PIf / 3;
		constexpr float _2pi = KSN_PIf * 2;

		//hue = fmodf(fabsf(hue), 2 * KSN_PIf);
		if (hue < 0) hue = -hue;
		if (hue > _2pi)
		{
			size_t int_part = size_t(hue / _2pi);
			hue -= _2pi * int_part;
		}
		if (hue <= pi3) return 1;
		if (hue < 2 * pi3) return (pi3 - hue) / pi3 + 1;
		if (hue <= 4 * pi3) return 0;
		if (hue < 5 * pi3) return (hue - 4 * pi3) / pi3 + 1;
		return 1;
	};
};

struct vertex2_t
{
	float x, y;
};
struct vertex3_t
{
	float x, y, z;
};
struct vertex4_t
{
	float x, y, z, w;
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
	ksn::fast_pimpl<_shape_buffer_impl, 4*4*sizeof(void*) + 2*sizeof(void*), sizeof(void*), true, true, true, true> m_impl;

public:

	shape_buffer_t() noexcept;
	shape_buffer_t(const shape_buffer_t&) noexcept;
	shape_buffer_t(shape_buffer_t&&) noexcept;
	~shape_buffer_t() noexcept;

	
	uint32_t registrate(const vertex2_t*, size_t, error_t* = nullptr) noexcept;
	uint32_t registrate(const vertex3_t*, size_t, error_t* = nullptr) noexcept;
	uint32_t registrate(const surface_vectorized_t*, size_t, error_t* = nullptr) noexcept;
	uint32_t registrate(const surface_indexed_t*, size_t, size_t vertex_index_offset, error_t* = nullptr) noexcept;
	uint32_t registrate(const texture_t*, size_t, error_t* = nullptr) noexcept;

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





class render_texture_t
{
	struct _render_texture_impl;
	ksn::fast_pimpl< _render_texture_impl, sizeof(void*) * 6, sizeof(void*), true, true, true, true> m_impl;

public:

	~render_texture_t() noexcept;
	render_texture_t() noexcept;
	render_texture_t(const render_texture_t&) noexcept = delete;
	render_texture_t(render_texture_t&&) noexcept;

	render_texture_t& operator=(const render_texture_t&) noexcept = delete;
	render_texture_t& operator=(render_texture_t&&) noexcept;
};


//b -> rb -> r -> rg -> g -> gb -> ...

_KSN_GRAPHICS_END

_KSN_END



#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif
