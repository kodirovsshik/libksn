
#include <ksn/ksn.hpp>

#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 4530) //False warning about exceptions enabled in <CL/OpenCL.hpp> but disabled in the project
#endif

#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110
//#include <CL/opencl.hpp>

#include <ksn/graphics_engine.hpp>



_KSN_BEGIN

_KSN_GRAPHICS_BEGIN



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






_KSN_GRAPHICS_END

_KSN_END
