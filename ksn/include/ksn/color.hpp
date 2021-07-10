
#ifndef _KSN_COLOR_HPP_
#define _KSN_COLOR_HPP_



#include <ksn/ksn.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/math_constexpr.hpp>



_KSN_BEGIN


struct color_bgra_t;

struct color_bgr_t
{
	uint8_t b, g, r;
	

	constexpr color_bgr_t() noexcept;
	constexpr color_bgr_t(color_bgra_t rgba) noexcept;
	constexpr color_bgr_t(uint8_t r, uint8_t g, uint8_t b) noexcept;
	constexpr color_bgr_t(uint32_t hex) noexcept;

	constexpr color_bgr_t& operator=(const color_bgr_t&) noexcept = default;
	constexpr color_bgr_t& operator=(color_bgr_t&&) noexcept = default;

	constexpr color_bgr_t& operator=(uint32_t hex) noexcept;

	constexpr explicit operator uint32_t() const noexcept;

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
	constexpr color_bgra_t(color_bgr_t rgb) noexcept;

	constexpr color_bgra_t(const color_bgra_t&) noexcept = default;
	constexpr color_bgra_t(color_bgra_t&&) noexcept = default;

	constexpr color_bgra_t& operator=(const color_bgra_t&) noexcept = default;
	constexpr color_bgra_t& operator=(color_bgra_t&&) noexcept = default;
	constexpr color_bgra_t& operator=(uint32_t hex) noexcept;

	constexpr explicit operator uint32_t() const noexcept;

	constexpr uint32_t rgba() const noexcept;
	constexpr uint32_t bgra() const noexcept;
};





struct color_hsv_t
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



	constexpr color_hsv_t() noexcept;
	constexpr color_hsv_t(color_bgra_t rgb) noexcept;
	constexpr color_hsv_t(uint16_t hue, uint8_t saturation, uint8_t value) noexcept;
	constexpr color_hsv_t(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept;

	constexpr color_hsv_t(const color_hsv_t&) noexcept = default;
	constexpr color_hsv_t(color_hsv_t&&) noexcept = default;

	constexpr color_bgra_t to_rgb() const noexcept;
	constexpr operator color_bgra_t() const noexcept;


private:

	//cos(x)+0.5 but bounded by y º [0; 1]
	constexpr static float limited_cos(float hue) noexcept;
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
constexpr color_bgr_t::color_bgr_t(color_bgra_t rgba) noexcept
	: b(rgba.b), g(rgba.g), r(rgba.r)
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

constexpr color_bgr_t& color_bgr_t::operator=(uint32_t hex) noexcept
{
	this->color_bgr_t::color_bgr_t(hex);
	return *this;
}

constexpr color_bgr_t::operator uint32_t() const noexcept
{
	return this->bgr();
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
	: color(hex | 0xFF000000)
{
}
constexpr color_bgra_t::color_bgra_t(uint32_t value, int unused) noexcept
	: color(value)
{
}
constexpr color_bgra_t::color_bgra_t(color_bgr_t rgb) noexcept
	: r(rgb.r), g(rgb.g), b(rgb.b), a(0xFF)
{
}

constexpr color_bgra_t& color_bgra_t::operator=(uint32_t hex) noexcept
{
	this->color = hex | 0xFF000000;
	return *this;
}

constexpr color_bgra_t::operator uint32_t() const noexcept
{
	return this->color;
}

constexpr uint32_t color_bgra_t::rgba() const noexcept
{
	return (this->color & 0xFF00FF00) | (this->r) | ((uint32_t)this->b << 16);
}
constexpr uint32_t color_bgra_t::bgra() const noexcept
{
	return this->color;
}





constexpr int16_t color_hsv_t::hue() const noexcept
{
	return (int16_t)(m_data & 1023);
}
constexpr uint8_t color_hsv_t::saturation() const noexcept
{
	return (uint8_t)((m_data & 131071) >> 10);
}
constexpr uint8_t color_hsv_t::value() const noexcept
{
	return (uint8_t)((m_data & 16777215) >> 17);
}
constexpr uint8_t color_hsv_t::alpha() const noexcept
{
	return (uint8_t)(m_data >> 24);
}
constexpr void color_hsv_t::hue(uint16_t hue) noexcept
{
	this->m_data = (this->m_data & 4294966272) | (hue & 1023);
}
constexpr void color_hsv_t::saturation(uint8_t saturation) noexcept
{
	this->m_data = (this->m_data & (uint32_t)4294837247) | (uint32_t(saturation & 127) << 10);
}
constexpr void color_hsv_t::value(uint8_t value) noexcept
{
	this->m_data = (this->m_data & (uint32_t)4278321151) | (uint32_t(value & 127) << 17);
}
constexpr void color_hsv_t::alpha(uint8_t alpha) noexcept
{
	this->m_data = (this->m_data & (uint32_t)16777215) | (uint32_t(alpha) << 24);
}



constexpr color_hsv_t::color_hsv_t() noexcept
{
	sizeof(*this);
	this->hue(0);
	this->saturation(0);
	this->value(0);
	this->alpha(255);
}
constexpr color_hsv_t::color_hsv_t(color_bgra_t rgb) noexcept
{
	uint8_t max_color = rgb.r;
	if (rgb.g > max_color) max_color = rgb.g;
	if (rgb.b > max_color) max_color = rgb.b;
	this->value(max_color * 100 / 255);

	max_color += max_color == 1;

	uint8_t max_diff = 0;
	//calculates the difference and updates max_diff if necessary
#define check_diff(a, b) { uint8_t temp = a >= b ? a - b : b - a; if (temp > max_diff) max_diff = temp; } ((void)0)
	check_diff(rgb.r, rgb.b);
	check_diff(rgb.r, rgb.g);
	check_diff(rgb.b, rgb.g);
#undef check_diff
	this->saturation(max_diff * 100 / max_color);


	uint16_t sum = rgb.r + rgb.b + rgb.g;
	sum += sum == 0;
	float hx = (rgb.r - (rgb.g + rgb.b) / 2.0f) / sum;
	float hy = (rgb.g - rgb.b) * ksn::sin(2 * KSN_PIf / 3) / sum;
	this->hue(int16_t(180 / KSN_PIf * ksn::fmod(ksn::atan2(hy, hx, 0.01f), 2 * KSN_PIf)));

	this->alpha(rgb.a);
}
constexpr color_hsv_t::color_hsv_t(uint16_t hue, uint8_t saturation, uint8_t value) noexcept
{
	this->m_data = (hue & 1023) | ((saturation & 127) << 10) | ((value & 127) << 17) | (-16777216);
}
constexpr color_hsv_t::color_hsv_t(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t alpha) noexcept
{
	this->m_data = (hue & 1023) | ((saturation & 127) << 10) | ((value & 127) << 17) | ((alpha) << 24);
}

constexpr color_bgra_t color_hsv_t::to_rgb() const noexcept
{
	color_bgra_t result;

	float max_color = 2.5500001f * this->value();
	float hue_rad = this->hue() * KSN_PIf / 180.0f;
	constexpr float pi3 = KSN_PIf / 3;

	result.r = uint8_t(max_color * (this->saturation() / 100.f * (limited_cos(hue_rad + -0 * pi3) - 1) + 1));
	result.g = uint8_t(max_color * (this->saturation() / 100.f * (limited_cos(hue_rad - 2 * pi3) - 1) + 1));
	result.b = uint8_t(max_color * (this->saturation() / 100.f * (limited_cos(hue_rad + 2 * pi3) - 1) + 1));
	result.a = this->alpha();
	return result;
}
constexpr color_hsv_t::operator color_bgra_t() const noexcept
{
	return this->to_rgb();
}

//cos(x)+0.5 but bounded by y º [0; 1]
constexpr float color_hsv_t::limited_cos(float hue) noexcept
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



_KSN_END



#endif //!_KSN_COLOR_HPP_
