
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
public:
	
	class _window_impl;



public:

#ifdef _WIN32
	#ifdef _WIN64
		ksn::fast_pimpl<_window_impl, 24, 8, true, true, true, true> m_impl;
	#else
		ksn::fast_pimpl<_window_impl, 12, 4, true, true, true, true> m_impl;
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

	union event_t
	{
		enum type_t : uint8_t
		{
			close = 0,
			move,
			resize,
			mouse_press,
			mouse_release,
			mouse_scroll,
			mouse_move,
			keyboard_press,
			keyboard_hold,
			keyboard_release,
			focus_gained,
			focus_lost,

			event_types_count
		};

		struct
		{
			enum button_t : uint8_t
			{
				left, right, wheel
			};

			uint16_t x, y;
			button_t button;
		} mouse_button_data;

		struct
		{
			enum button_t : uint8_t
			{
				unknown = uint8_t(-1),
				a = 0,
				b,
				c,
				d,
				e,
				f,
				g,
				h,
				i,
				j,
				k,
				l,
				m,
				o,
				p,
				q,
				r,
				s,
				t,
				u,
				v,
				w,
				x,
				y,
				z,

				shift_left,
				shift_right,

				ctrl_left,
				ctrl_right,

				alt_left,
				alt_right,

				super_left,
				super_right,

				num_lock,
				caps_lock,
				scroll_lock,

				tab,
				esc,
				tilde,
				space,

				digit0,
				digit1,
				digit2,
				digit3,
				digit4,
				digit5,
				digit6,
				digit7,
				digit8,
				digit9,

				numpad0,
				numpad1,
				numpad2,
				numpad3,
				numpad4,
				numpad5,
				numpad6,
				numpad7,
				numpad8,
				numpad9,

				F1,
				F2,
				F3,
				F4,
				F5,
				F6,
				F7,
				F8,
				F9,
				F10,
				F11,
				F12,

				insert,
				delete_,
				home,
				end,
				page_up,
				page_down,

				arrow_up,
				arrow_down,
				arrow_left,
				arrow_right,

				print_screen,
				break_,
				pause = break_,

				buttons_count
			};

			button_t button;
		} keyboard_button_data;

		struct
		{
			uint16_t x_new, y_new;
			uint16_t x_old, y_old;
		} window_move_data;

		struct
		{
			uint16_t width_new, height_new;
			uint16_t width_old, height_old;
		} window_resize_data;
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