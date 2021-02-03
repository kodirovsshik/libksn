
#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>



_KSN_BEGIN


/*
Tech notes:


*/

/*
b


*/
class graphics_engine_t
{
private:

	struct _graphics_engine_impl;
	fast_pimpl<_graphics_engine_impl, 1, 1> m_impl;


public:
	graphics_engine_t() noexcept = default;
	graphics_engine_t(const graphics_engine_t&) noexcept = delete;
	graphics_engine_t(graphics_engine_t&&) noexcept = default;
	~graphics_engine_t() noexcept = default;

	graphics_engine_t& operator=(const graphics_engine_t&) noexcept = delete;
	graphics_engine_t& operator=(graphics_engine_t&&) noexcept = default;



};

union color_t
{
	struct
	{
		uint8_t b, g, r, a;
	};
	uint32_t color;

	constexpr color_t() noexcept;
	constexpr color_t(uint8_t r, uint8_t g, uint8_t b) noexcept;
	constexpr color_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;
	constexpr color_t(uint32_t) noexcept;

};

struct vertex2_t
{
	float x, y;
};
struct vertex3_t
{
	float x, y, z;
};

struct surface_t
{
	vertex3_t v[3];
};

//b -> rb -> r -> rg -> g -> gb -> ...

_KSN_END
