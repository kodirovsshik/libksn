
#ifndef _KSN_GRAPHICS_ENGINE_WINDOW_HPP_
#define _KSN_GRAPHICS_ENGINE_WINDOW_HPP_


#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>
#include <Windows.h>

_KSN_BEGIN



class window_t
{
private:

	struct _window_impl;

	struct Event
	{
		enum type_t
		{
			create,
			count,
		};
	};

	using my_t = window_t;



public:

#ifdef _KSN_COMPILER_MSVC
#if _KSN_IS_64
	ksn::fast_pimpl<my_t::_window_impl, 24, 8> m_impl;
#else
	ksn::fast_pimpl<my_t::_window_impl, 12, 4> m_impl;
#endif
#else
#error !!
#endif

public:

	enum style_t : uint8_t
	{
		border = 1,
		close_button = 2,
		close_min_max = 4,
		resize = 8,
		caption = 16,
		fullscreen = 32,

		default_style = border | close_min_max | resize | caption
	};

	struct context_settings
	{
		uint8_t bits_per_color = 24;
		uint8_t ogl_version_major = 1;
		uint8_t ogl_version_minor = 1;
		bool ogl_compatibility_profile : 1 = false;
		bool ogl_debug : 1 = false;
	};



	window_t() noexcept;
	window_t(const window_t&) = delete;
	window_t(window_t&&);

	window_t& operator=(const window_t&) = delete;
	window_t& operator=(window_t&&);

	~window_t() noexcept;


	window_t(size_t width, size_t height, const char* title = "", context_settings settings = {}, style_t style = style_t::default_style) noexcept;
	window_t(size_t width, size_t height, const wchar_t* title, context_settings settings = {}, style_t style = style_t::default_style) noexcept;

	bool open(size_t width, size_t height, const char* title = "", context_settings settings = {}, style_t style = style_t::default_style) noexcept;
	bool open(size_t width, size_t height, const wchar_t* title, context_settings settings = {}, style_t style = style_t::default_style) noexcept;

	void close() noexcept;

	bool poll_event(MSG&);
};


_KSN_END

#endif //!_KSN_GRAPHICS_ENGINE_WINDOW_HPP_
