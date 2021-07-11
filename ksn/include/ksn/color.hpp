
//TODO: conversions

#ifndef _KSN_COLOR_HPP_
#define _KSN_COLOR_HPP_



#include <ksn/ksn.hpp>
#include <ksn/color_defs.hpp>



_KSN_BEGIN


struct color_bgr_t
{
	uint8_t b, g, r;
	

	constexpr color_bgr_t() noexcept;
	constexpr color_bgr_t(uint8_t r, uint8_t g, uint8_t b) noexcept;
	constexpr color_bgr_t(uint32_t hex) noexcept;
	constexpr color_bgr_t(detail::_color_generic_value) noexcept;

	constexpr color_bgr_t& operator=(const color_bgr_t&) noexcept = default;
	constexpr color_bgr_t& operator=(color_bgr_t&&) noexcept = default;

	constexpr uint32_t rgb() const noexcept;
	constexpr uint32_t bgr() const noexcept;


	constexpr color_bgr_t(const color_bgr_t&) noexcept = default;
	constexpr color_bgr_t(color_bgr_t&&) noexcept = default;
};



struct color_bgra_t
{
	union
	{
		struct
		{
			uint8_t b, g, r, a;
		};
		uint32_t color;
	};

	constexpr color_bgra_t() noexcept;
	constexpr color_bgra_t(uint8_t r, uint8_t g, uint8_t b) noexcept;
	constexpr color_bgra_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;
	constexpr color_bgra_t(uint32_t hex) noexcept;
	constexpr color_bgra_t(uint32_t value, int unused) noexcept;
	constexpr color_bgra_t(detail::_color_generic_value) noexcept;

	constexpr color_bgra_t(const color_bgra_t&) noexcept = default;
	constexpr color_bgra_t(color_bgra_t&&) noexcept = default;

	constexpr color_bgra_t& operator=(const color_bgra_t&) noexcept = default;
	constexpr color_bgra_t& operator=(color_bgra_t&&) noexcept = default;

	constexpr uint32_t rgba() const noexcept;
	constexpr uint32_t bgra() const noexcept;
};



struct color_rgb_t
{
	uint8_t r, g, b;


	constexpr color_rgb_t() noexcept;
	constexpr color_rgb_t(uint8_t r, uint8_t g, uint8_t b) noexcept;
	constexpr color_rgb_t(uint32_t hex) noexcept;
	constexpr color_rgb_t(detail::_color_generic_value) noexcept;

	constexpr color_rgb_t(const color_rgb_t&) noexcept = default;
	constexpr color_rgb_t(color_rgb_t&&) noexcept = default;

	constexpr color_rgb_t& operator=(const color_rgb_t&) noexcept = default;
	constexpr color_rgb_t& operator=(color_rgb_t&&) noexcept = default;

	constexpr uint32_t rgb() const noexcept;
	constexpr uint32_t bgr() const noexcept;
};



struct color_rgba_t
{
	union
	{
		struct
		{
			uint8_t r, g, b, a;
		};
		uint32_t color;
	};

	constexpr color_rgba_t() noexcept;
	constexpr color_rgba_t(uint8_t r, uint8_t g, uint8_t b) noexcept;
	constexpr color_rgba_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;
	constexpr color_rgba_t(uint32_t hex) noexcept;
	constexpr color_rgba_t(uint32_t value, int unused) noexcept;
	constexpr color_rgba_t(detail::_color_generic_value) noexcept;

	constexpr color_rgba_t(const color_rgba_t&) noexcept = default;
	constexpr color_rgba_t(color_rgba_t&&) noexcept = default;

	constexpr color_rgba_t& operator=(const color_rgba_t&) noexcept = default;
	constexpr color_rgba_t& operator=(color_rgba_t&&) noexcept = default;

	constexpr uint32_t rgba() const noexcept;
	constexpr uint32_t bgra() const noexcept;
};



struct color_hsva_t
{
	//uint16_t hue : 10; //0 - 359
	//uint8_t saturation : 7; //0-100
	//uint8_t value : 7; //0-100
	//uint8_t alpha; //0-255

	//What was the point of introducing a bit fields in the language if they are not packed

	uint32_t m_data;



	constexpr int16_t hue() const noexcept;
	constexpr uint8_t saturation() const noexcept;
	constexpr uint8_t value() const noexcept;
	constexpr uint8_t alpha() const noexcept;

	constexpr void hue(uint16_t hue) noexcept;
	constexpr void saturation(uint8_t saturation) noexcept;
	constexpr void value(uint8_t value) noexcept;
	constexpr void alpha(uint8_t alpha) noexcept;



	constexpr color_hsva_t() noexcept;
	constexpr color_hsva_t(uint16_t hue, uint8_t saturation, uint8_t value) noexcept;
	constexpr color_hsva_t(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept;
	constexpr color_hsva_t(detail::_color_generic_value) noexcept;

	constexpr color_hsva_t(const color_hsva_t&) noexcept = default;
	constexpr color_hsva_t(color_hsva_t&&) noexcept = default;

};


_KSN_END






//////////////////////////////////////
//
//			Implementation
//
/////////////////////////////////////






_KSN_BEGIN

//Fourtenately, constexpr for functions does imply it being inline
//Thus including the header in many sourcefiles sould not cause an error
//N4860, §6.3/13.3


constexpr color_bgr_t::color_bgr_t() noexcept
	: b(0), g(0), r(0)
{
}
constexpr color_bgr_t::color_bgr_t(uint8_t r, uint8_t g, uint8_t b) noexcept
	: b(b), g(g), r(r)
{
}
constexpr color_bgr_t::color_bgr_t(uint32_t hex) noexcept
	: b((uint8_t)hex), g(uint8_t(hex >> 8)), r(uint8_t(hex >> 16))
{
}

constexpr color_bgr_t::color_bgr_t(detail::_color_generic_value x) noexcept
	: b(x.b), g(x.g), r(x.r)
{
}

constexpr uint32_t color_bgr_t::rgb() const noexcept
{
	return this->r | ((uint32_t)this->g << 8) | ((uint32_t)this->b << 16);
}
constexpr uint32_t color_bgr_t::bgr() const noexcept
{
	return this->b | ((uint32_t)this->g << 8) | ((uint32_t)this->r << 16);
}





constexpr color_bgra_t::color_bgra_t() noexcept
	: color(0xFF000000)
{
}
constexpr color_bgra_t::color_bgra_t(uint8_t r, uint8_t g, uint8_t b) noexcept
	: r(r), g(g), b(b), a(0xFF)
{
}
constexpr color_bgra_t::color_bgra_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a)  noexcept
	: r(r), g(g), b(b), a(a)
{
}
constexpr color_bgra_t::color_bgra_t(uint32_t hex) noexcept
	: color(hex)
{
	this->a = 0xFF;
}
constexpr color_bgra_t::color_bgra_t(uint32_t value, int) noexcept
	: color(value)
{
}

constexpr color_bgra_t::color_bgra_t(detail::_color_generic_value x) noexcept
	: color(x.m_value)
{
}

constexpr uint32_t color_bgra_t::rgba() const noexcept
{
	return (this->color & 0xFF00FF00) | (this->r) | ((uint32_t)this->b << 16);
}
constexpr uint32_t color_bgra_t::bgra() const noexcept
{
	return this->color;
}





constexpr color_rgb_t::color_rgb_t() noexcept
	: r(0), g(0), b(0)
{
}
constexpr color_rgb_t::color_rgb_t(uint8_t r, uint8_t g, uint8_t b) noexcept
	: r(r), g(g), b(b)
{
}
constexpr color_rgb_t::color_rgb_t(uint32_t hex) noexcept
	: r(uint8_t(hex >> 16)), g(uint8_t(hex >> 8)), b((uint8_t)hex)
{
}
constexpr color_rgb_t::color_rgb_t(detail::_color_generic_value x) noexcept
	: r(x.r), g(x.g), b(x.b)
{
}

constexpr uint32_t color_rgb_t::rgb() const noexcept
{
	return
		((uint32_t)this->b) |
		((uint32_t)this->g >> 8) |
		((uint32_t)this->r >> 16);
}
constexpr uint32_t color_rgb_t::bgr() const noexcept
{
	return
		((uint32_t)this->b >> 16) |
		((uint32_t)this->g >> 8) |
		((uint32_t)this->r);
}





constexpr color_rgba_t::color_rgba_t() noexcept
	: color(0)
{
}
constexpr color_rgba_t::color_rgba_t(uint8_t r, uint8_t g, uint8_t b) noexcept
	: r(r), g(g), b(b), a(0xFF)
{
}
constexpr color_rgba_t::color_rgba_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
	: r(r), g(g), b(b), a(a)
{
}
constexpr color_rgba_t::color_rgba_t(uint32_t hex) noexcept
	: color(hex)
{
}
constexpr color_rgba_t::color_rgba_t(uint32_t value, int unused) noexcept
	: b((uint8_t)value), g(uint8_t(value >> 8)), r(uint8_t(value >> 16)), a(value >> 24)
{
}
constexpr color_rgba_t::color_rgba_t(detail::_color_generic_value x) noexcept
	: b((uint8_t)x.m_value), g(uint8_t(x.m_value >> 8)), r(uint8_t(x.m_value >> 16)), a(x.m_value >> 24)
{
}

constexpr uint32_t color_rgba_t::rgba() const noexcept
{
	return this->color;
}
constexpr uint32_t color_rgba_t::bgra() const noexcept
{
	return detail::_color_generic_value::from_rgba(this->color).m_value;
}





constexpr int16_t color_hsva_t::hue() const noexcept
{
	return (int16_t)(m_data & 1023);
}
constexpr uint8_t color_hsva_t::saturation() const noexcept
{
	return (uint8_t)((m_data & 131071) >> 10);
}
constexpr uint8_t color_hsva_t::value() const noexcept
{
	return (uint8_t)((m_data & 16777215) >> 17);
}
constexpr uint8_t color_hsva_t::alpha() const noexcept
{
	return (uint8_t)(m_data >> 24);
}

constexpr void color_hsva_t::hue(uint16_t hue) noexcept
{
	this->m_data = (this->m_data & 4294966272) | (hue & 1023);
}
constexpr void color_hsva_t::saturation(uint8_t saturation) noexcept
{
	this->m_data = (this->m_data & (uint32_t)4294837247) | (uint32_t(saturation & 127) << 10);
}
constexpr void color_hsva_t::value(uint8_t value) noexcept
{
	this->m_data = (this->m_data & (uint32_t)4278321151) | (uint32_t(value & 127) << 17);
}
constexpr void color_hsva_t::alpha(uint8_t alpha) noexcept
{
	this->m_data = (this->m_data & (uint32_t)16777215) | (uint32_t(alpha) << 24);
}



constexpr color_hsva_t::color_hsva_t() noexcept
{
	sizeof(*this);
	this->hue(0);
	this->saturation(0);
	this->value(0);
	this->alpha(255);
}
constexpr color_hsva_t::color_hsva_t(uint16_t hue, uint8_t saturation, uint8_t value) noexcept
{
	this->m_data = (hue & 1023) | ((saturation & 127) << 10) | ((value & 127) << 17) | (-16777216);
}
constexpr color_hsva_t::color_hsva_t(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept
{
	this->m_data = (hue & 1023) | ((saturation & 127) << 10) | ((value & 127) << 17) | ((alpha) << 24);
}
constexpr color_hsva_t::color_hsva_t(detail::_color_generic_value x) noexcept
{
	uint8_t max_color = x.r;
	if (x.g > max_color) max_color = x.g;
	if (x.b > max_color) max_color = x.b;
	this->value(max_color * 100 / 255);

	max_color += max_color == 1;

	uint8_t max_diff = 0;
	//calculates the difference and updates max_diff if necessary
#define check_diff(a, b) { uint8_t temp = a >= b ? a - b : b - a; if (temp > max_diff) max_diff = temp; } ((void)0)
	check_diff(x.r, x.b);
	check_diff(x.r, x.g);
	check_diff(x.b, x.g);
#undef check_diff
	this->saturation(max_diff * 100 / max_color);


	uint16_t sum = x.r + x.b + x.g;
	sum += sum == 0;
	float hx = (x.r - (x.g + x.b) / 2.0f) / sum;
	float hy = (x.g - x.b) * ksn::sin(2 * KSN_PIf / 3) / sum;
	this->hue(int16_t(180 / KSN_PIf * ksn::fmod(ksn::atan2(hy, hx, 0.01f), 2 * KSN_PIf)));

	this->alpha(x.a);
}




_KSN_END



#endif //!_KSN_COLOR_HPP_
