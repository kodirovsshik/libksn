
#include <ksn/ksn.hpp>

#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 4530) //False warning about exceptions enabled in <CL/OpenCL.hpp> but disabled in the project
#endif

#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110
#include <CL/opencl.hpp>

#include <ksn/graphics_engine.hpp>



_KSN_BEGIN


cl::Context ge_cl_context;
int ge_cl_platform_number = 0; //0 stands for none, use native CPU (but try extensions)
std::vector<std::pair<cl::Platform, std::vector<cl::Device>>> ge_cl_impl;


//constexpr color_t::color_t() noexcept
//	: color(0xFF000000)
//{
//}
//constexpr color_t::color_t(uint8_t r_, uint8_t g_, uint8_t b_) noexcept
//	: a(0), r(r_), g(g_), b(b_)
//{
//}
//constexpr color_t::color_t(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept
//	: a(a_), r(r_), g(g_), b(b_)
//{
//}
//constexpr color_t::color_t(uint32_t _hex) noexcept
//	: color((_hex & 0xFFFFFF) | 0x000000FF)
//{
//}
//constexpr color_t::color_t(uint32_t value, int) noexcept
//	: color(value)
//{
//}
//
//constexpr color_t& color_t::operator=(uint32_t value) noexcept
//{
//	this->color = (value & 0x00FFFFFF) | (uint32_t(this->a) << 24);
//	return *this;
//}



struct graphics_engine_t::_graphics_engine_impl
{
	_graphics_engine_impl() noexcept
	{

	}
	_graphics_engine_impl(const _graphics_engine_impl&) = delete;
	_graphics_engine_impl(_graphics_engine_impl&&) noexcept
	{

	}
	~_graphics_engine_impl() noexcept
	{

	}

	_graphics_engine_impl& operator=(const _graphics_engine_impl&) = delete;
	_graphics_engine_impl& operator=(_graphics_engine_impl&&) noexcept
	{
		return *this;
	}
};

graphics_engine_t::graphics_engine_t() noexcept 
{
}
graphics_engine_t::graphics_engine_t(graphics_engine_t&& other) noexcept 
	: m_impl(std::move(other.m_impl))
{
}
graphics_engine_t::~graphics_engine_t() noexcept
{
}
graphics_engine_t& graphics_engine_t::operator=(graphics_engine_t&& other) noexcept
{
	std::iter_swap(this->m_impl.ptr(), other.m_impl.ptr());
	return *this;
}






_KSN_END
