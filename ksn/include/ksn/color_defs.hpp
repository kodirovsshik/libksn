
#ifndef _KSN_COLOR_DEFS_HPP_
#define _KSN_COLOR_DEFS_HPP_



#include <ksn/ksn.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/math_constexpr.hpp>
#include <ksn/metapr.hpp>





_KSN_BEGIN


struct color_rgb_t;
struct color_rgba_t;
struct color_bgr_t;
struct color_bgra_t;
struct color_hsv_t;
struct color_hsv3_t;
struct color_hsva_t;


template<class color_t>
concept color = is_any_of_v<color_t, color_rgb_t, color_rgba_t, color_bgr_t, color_bgra_t, color_hsv_t, color_hsv3_t, color_hsva_t>;



_KSN_DETAIL_BEGIN


//BGRA, used as an intermediate type to convert between different color types
struct _color_generic_value
{
private:
	constexpr _color_generic_value() noexcept;
	static constexpr _color_generic_value _hsva_to_generic(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept;

public:

	constexpr _color_generic_value(uint32_t value) noexcept;


	static constexpr _color_generic_value from_rgb(uint8_t r, uint8_t g, uint8_t b) noexcept;

	//Memory layout: R, G, B
	static constexpr _color_generic_value from_rgb(uint32_t mem_layout_rgb) noexcept;



	static constexpr _color_generic_value from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;

	//Memory layout: R, G, B, A
	static constexpr _color_generic_value from_rgba(uint32_t mem_layout_rgba) noexcept;



	static constexpr _color_generic_value from_bgr(uint8_t b, uint8_t g, uint8_t r) noexcept;

	//Memory layout: B, G, R
	static constexpr _color_generic_value from_bgr(uint32_t mem_layout_bgr) noexcept;



	static constexpr _color_generic_value from_bgra(uint8_t b, uint8_t g, uint8_t r, uint8_t a) noexcept;

	//Memory layout: B, G, R, A
	static constexpr _color_generic_value from_bgra(uint32_t mem_layout_bgra) noexcept;



	static constexpr _color_generic_value from_hsv(uint16_t hue, uint8_t saturation, uint8_t value) noexcept;

	//Memory layout: H[2], S, V
	static constexpr _color_generic_value from_hsv(uint32_t mem_layout_hsv) noexcept;



	static constexpr _color_generic_value from_hsva(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept;

	//Memory layout: H[10 bits], S[7 bits], V[7 bits], A[8 bits]
	//32 bits in total, 4 bytes
	static constexpr _color_generic_value from_hsva(uint32_t mem_layout_hsva_packed) noexcept;

	//Memory layout: H[2], S, V, A
	//5 bytes in total
	static constexpr _color_generic_value from_hsva(const uint8_t* data) noexcept;
	
	
	
	static constexpr _color_generic_value from_color(color_bgr_t) noexcept;
	static constexpr _color_generic_value from_color(color_bgra_t) noexcept;
	static constexpr _color_generic_value from_color(color_rgb_t) noexcept;
	static constexpr _color_generic_value from_color(color_rgba_t) noexcept;
	static constexpr _color_generic_value from_color(color_hsv_t) noexcept;
	static constexpr _color_generic_value from_color(color_hsv3_t) noexcept;
	static constexpr _color_generic_value from_color(color_hsva_t) noexcept;





	union
	{
		uint32_t m_value;
		uint8_t byte[4];
		struct
		{
			uint8_t b, g, r, a;
		};
	};
};


_KSN_DETAIL_END


_KSN_END





////////////////////////////////
//
//		Implementation
//
///////////////////////////////





_KSN_BEGIN

_KSN_DETAIL_BEGIN


#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

constexpr _color_generic_value::_color_generic_value() noexcept {}

constexpr _color_generic_value::_color_generic_value(uint32_t value) noexcept : m_value(value) {}


#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif





constexpr _color_generic_value _color_generic_value::from_rgb(uint8_t r, uint8_t g, uint8_t b) noexcept
{
	_color_generic_value result;
	result.b = b;
	result.g = g;
	result.r = r;
	result.a = 0xFF;
	return result;
}
constexpr _color_generic_value _color_generic_value::from_rgb(uint32_t mem_layout_rgb) noexcept
{
	_color_generic_value result;
	result.b = result.r;
	result.g = result.g;
	result.r = result.b;
	result.a = 0xFF;
	return result;
}



constexpr _color_generic_value _color_generic_value::from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
{
	_color_generic_value result;
	result.b = b;
	result.g = g;
	result.r = r;
	result.a = a;
	return result;
}
constexpr _color_generic_value _color_generic_value::from_rgba(uint32_t mem_layout_rgba) noexcept
{
	_color_generic_value result;
	result.b = ((uint8_t*)&mem_layout_rgba)[2];
	result.g = ((uint8_t*)&mem_layout_rgba)[1];
	result.r = ((uint8_t*)&mem_layout_rgba)[0];
	result.a = ((uint8_t*)&mem_layout_rgba)[3];
	return result;
}



constexpr _color_generic_value _color_generic_value::from_bgr(uint8_t b, uint8_t g, uint8_t r) noexcept
{
	_color_generic_value result;
	result.b = b;
	result.g = g;
	result.r = r;
	result.a = 0xFF;
	return result;
}
constexpr _color_generic_value _color_generic_value::from_bgr(uint32_t mem_layout_bgr) noexcept
{
	_color_generic_value result;
	result.b = ((uint8_t*)&mem_layout_bgr)[0];
	result.g = ((uint8_t*)&mem_layout_bgr)[1];
	result.r = ((uint8_t*)&mem_layout_bgr)[2];
	result.a = 0xFF;
	return result;
}

constexpr _color_generic_value _color_generic_value::from_bgra(uint8_t b, uint8_t g, uint8_t r, uint8_t a) noexcept
{
	_color_generic_value result;
	result.b = b;
	result.g = g;
	result.r = r;
	result.a = a;
	return result;
}
constexpr _color_generic_value _color_generic_value::from_bgra(uint32_t mem_layout_bgra) noexcept
{
	return _color_generic_value{ mem_layout_bgra };
}

constexpr float limited_cos(float hue) noexcept
{
	constexpr float pi3 = KSN_PIf / 3;
	constexpr float _2pi = KSN_PIf * 2;

	if (hue < 0) hue = -hue;
	if (hue > _2pi) hue -= _2pi * size_t(hue / _2pi);
	if (hue <= pi3) return 1;
	if (hue < 2 * pi3) return (pi3 - hue) / pi3 + 1;
	if (hue <= 4 * pi3) return 0;
	if (hue < 5 * pi3) return (hue - 4 * pi3) / pi3 + 1;
	return 1;
};


constexpr _color_generic_value _color_generic_value::_hsva_to_generic(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept
{
	_color_generic_value result;

	float max_color = 2.5500001f * value;
	float hue_rad = hue * KSN_PIf / 180.0f;
	constexpr float pi3 = KSN_PIf / 3;

	result.r = uint8_t(max_color * (saturation / 100.f * (limited_cos(hue_rad + -0 * pi3) - 1) + 1));
	result.g = uint8_t(max_color * (saturation / 100.f * (limited_cos(hue_rad - 2 * pi3) - 1) + 1));
	result.b = uint8_t(max_color * (saturation / 100.f * (limited_cos(hue_rad + 2 * pi3) - 1) + 1));
	result.a = alpha;

	return result;
}


constexpr _color_generic_value _color_generic_value::from_hsv(uint16_t hue, uint8_t saturation, uint8_t value) noexcept
{
	return _hsva_to_generic(hue, saturation, value, 0xFF);
}
constexpr _color_generic_value _color_generic_value::from_hsv(uint32_t mem_layout_hsv) noexcept
{
	return _hsva_to_generic(
		(uint16_t)mem_layout_hsv,
		((uint8_t*)&mem_layout_hsv)[2],
		((uint8_t*)&mem_layout_hsv)[3],
		0xFF);
}

constexpr _color_generic_value _color_generic_value::from_hsva(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept
{
	return _hsva_to_generic(hue, saturation, value, alpha);
}
constexpr _color_generic_value _color_generic_value::from_hsva(uint32_t mem_layout_hsva_packed) noexcept
{
	uint16_t hue = uint16_t(mem_layout_hsva_packed & 0b1111111111);
	uint8_t saturation = uint8_t((mem_layout_hsva_packed >> 10) & 0b1111111);
	uint8_t value = uint8_t((mem_layout_hsva_packed >> 17) & 1111111);
	uint8_t alpha = uint8_t(mem_layout_hsva_packed >> 24);

	return _hsva_to_generic(hue, saturation, value, alpha);
}
constexpr _color_generic_value _color_generic_value::from_hsva(const uint8_t* data) noexcept
{
	uint16_t hue = *(uint16_t*)data;
	uint8_t saturation = data[2];
	uint8_t value = data[3];
	uint8_t alpha = data[4];

	return _hsva_to_generic(hue, saturation, value, alpha);
}

_KSN_DETAIL_END

_KSN_END



#endif //!_KSN_COLOR_DEFS_HPP_
