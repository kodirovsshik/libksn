
#ifndef _KSN_WINDOW_GL_HPP_
#define _KSN_WINDOW_GL_HPP_


#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>
#include <ksn/window.hpp>



#ifdef _WIN32

struct HGLRC__;

#endif



_KSN_BEGIN


class window_gl_t
	: public window_t
{
public:

	class _window_gl_impl;


#if defined _WIN32
	ksn::fast_pimpl<_window_gl_impl, sizeof(void*), alignof(void*)> m_gl_impl;
#else

#error This platform is not (possibly yet) supported by <ksn/window_gl.hpp>

#endif



public:
#ifdef _WIN32
	using native_context_t = HGLRC__*;
#endif

	native_context_t context_native_handle() const noexcept;


	struct context_settings
	{
		uint8_t ogl_version_major = 1;
		uint8_t ogl_version_minor = 1;
		uint8_t bits_per_color = 24;
		bool ogl_compatibility_profile : 1 = true;
		bool ogl_debug : 1 = false;
	};

	static constexpr context_settings opengl_no_context{ 0, 0, 0, 0, 0 };
	static constexpr context_settings opengl_default_context{};



	window_gl_t() noexcept;
	window_gl_t(const window_gl_t&) = delete;
	window_gl_t(window_gl_t&&) noexcept;
	~window_gl_t() noexcept;

	window_gl_t(uint16_t width, uint16_t height, const char* title = "", context_settings settings = {}, window_style_t style = window_style::default_style) noexcept;
	window_gl_t(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings = {}, window_style_t style = window_style::default_style) noexcept;

	window_open_result_t open(uint16_t width, uint16_t height, const char* title = "", context_settings settings = {}, window_style_t style = window_style::default_style) noexcept;
	window_open_result_t open(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings = {}, window_style_t style = window_style::default_style) noexcept;



	void context_make_current() const noexcept;
	bool context_is_current() const noexcept;

	void close() noexcept;

	bool context_present() const noexcept;
};


_KSN_END



#endif //!_KSN_WINDOW_GL_HPP_
