
#ifndef _KSN_WINDOW_HPP_
#define _KSN_WINDOW_HPP_


#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>



#ifdef _WIN32

struct HWND__;
//struct HDC__;
struct HGLRC__;
struct tagMSG;

#endif



_KSN_BEGIN


class window_t
{
private:
	
	class _window_impl;



public:

#ifdef _WIN32
	#ifdef _WIN64
		ksn::fast_pimpl<_window_impl, 32, 8, true, true, true, true> m_impl;
	#else
		ksn::fast_pimpl<_window_impl, 16, 4, true, true, true, true> m_impl;
	#endif
#else

#error This platform is not (possibly yet) supported by <ksn/window.hpp>

#endif

	using native_window_t = HWND__*;
	using native_context_t = HGLRC__*;
	using native_event_t = tagMSG;



	struct context_settings
	{
		uint8_t ogl_version_major = 1;
		uint8_t ogl_version_minor = 1;
		uint8_t bits_per_color = 24;
		bool ogl_compatibility_profile : 1 = false;
		bool ogl_debug : 1 = false;
	};

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

	struct event_t
	{
		enum type_t : uint8_t
		{
			create = 0,
			close = 1,
			move = 2,
		};
	};

	enum error_t : uint8_t
	{
		//All fine, window is opened
		ok = 0,

		//Not (possibly yet) implemented by the library
		unimplemented = 1,

		//Error after calling system API
		//(except for window creation function itself)
		system_error = 2,

		//Error after calling OpenGL API
		opengl_error = 3,

		//Window creation system function failed
		window_creation_error = 4,

		//Invalid window size (too big/small)
		window_size_error = 5,

		//Failed to initialize GLEW
		glew_error = 6,

		//OpenGL feature required is not supported
		opengl_unsupported_function = 7
	};

	





	native_window_t window_native_handle() const noexcept;
	native_context_t context_native_handle() const noexcept;

	window_t() noexcept;
	window_t(const window_t&) = delete;
	window_t(window_t&&) noexcept;
	~window_t() noexcept;

	window_t(uint16_t width, uint16_t height, const char* title = "", context_settings settings = {}, style_t style = style_t::default_style) noexcept;
	window_t(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings = {}, style_t style = style_t::default_style) noexcept;

	error_t open(uint16_t width, uint16_t height, const char* title = "", context_settings settings = {}, style_t style = style_t::default_style) noexcept;
	error_t open(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings = {}, style_t style = style_t::default_style) noexcept;

	void close() noexcept;

	bool poll_event(event_t&) const noexcept;
	bool wait_event(event_t&) const noexcept;

	bool poll_native_event(native_event_t&) const noexcept;
	bool wait_native_event(native_event_t&) const noexcept;

	bool is_open() const noexcept;

	void make_current() const noexcept;
	bool is_current() const noexcept;

};


_KSN_END



#endif //!_KSN_WINDOW_HPP_