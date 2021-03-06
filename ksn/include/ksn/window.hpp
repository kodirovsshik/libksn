
#ifndef _KSN_WINDOW_HPP_
#define _KSN_WINDOW_HPP_



#include <ksn/ksn.hpp>
#include <ksn/fast_pimpl.hpp>



#ifdef _WIN32

struct HWND__;
struct HDC__;
struct tagMSG;

#endif



_KSN_BEGIN



enum class event_type_t : uint8_t
{
	create = 0,
	open = create,
	close,
	move,
	resize,
	mouse_press,
	mouse_release,
	mouse_scroll,
	mouse_scroll_vertical = mouse_scroll,
	mouse_scroll_horizontal,
	mouse_move,
	keyboard_press,
	keyboard_hold,
	keyboard_release,
	focus_gained,
	focus_lost,
	mouse_entered,
	mouse_leave,
	maximized,
	minimized,
	text,

	event_types_count
};



enum class mouse_button_t : uint8_t
{
	left, right, middle, extra1, extra2
};

enum class keyboard_button_t : uint8_t
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
	n,
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
	control_left = ctrl_left,
	ctrl_right,
	control_right = ctrl_right,

	alt_left,
	alt_right,

	system_left,
	system_right,

	//context menu key
	menu_left, 
	//context menu key
	menu_right, 

	enter,
	backspace,

	minus,
	hypen = minus,
	equal,
	equal_sign = equal,
	bracket_left,
	bracket_right,
	semicolon,
	quote,
	backslash,
	comma,
	period,
	slash,

	num_lock,
	caps_lock,
	scroll_lock,

	tab,
	esc,
	escape = esc,
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

	add,
	substract,
	multiply,
	divide,

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
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,
		

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

	other,

	buttons_count
};



struct event_t
{

	event_type_t type;

	union
	{
		struct
		{
			mouse_button_t button;
			uint16_t x, y;
		} mouse_button_data;

		struct
		{
			float delta;
			uint16_t x, y; //Cursor position
			bool is_vertical;
		} mouse_scroll_data;

		struct
		{
			keyboard_button_t button;

			//Whether it was pressed with the reported button
			bool alt, control, shift, system;

		} keyboard_button_data;

		struct
		{
			int32_t x_new, y_new;
			int32_t x_old, y_old;
		} window_move_data;

		struct
		{
			uint16_t width_new, height_new;
			uint16_t width_old, height_old;
		} window_resize_data;

		struct
		{
			int32_t x, y;
		} mouse_move_data;

		char32_t character;
	};

	const char* to_string() const noexcept
	{
		switch (this->type)
		{
		case event_type_t::create: return "create";
		case event_type_t::close: return "close";
		case event_type_t::move: return "move";
		case event_type_t::resize: return "resize";
		case event_type_t::mouse_press: return "mouse_press";
		case event_type_t::mouse_release: return "mouse_release";
		case event_type_t::mouse_scroll_vertical: return "mouse_scroll_vertical";
		case event_type_t::mouse_scroll_horizontal: return "mouse_scroll_horizontal";
		case event_type_t::mouse_move: return "mouse_move";
		case event_type_t::keyboard_press: return "keyboard_press";
		case event_type_t::keyboard_hold: return "keyboard_hold";
		case event_type_t::keyboard_release: return "keyboard_release";
		case event_type_t::focus_gained: return "focus_gained";
		case event_type_t::focus_lost: return "focus_lost";
		case event_type_t::mouse_entered: return "mouse_entered";
		case event_type_t::mouse_leave: return "mouse_leave";
		case event_type_t::maximized: return "maximized";
		case event_type_t::minimized: return "minimized";
		case event_type_t::text: return "text";
		default: return "<corrupted event>";
		}
	}

	/*void dump(FILE* f = stdout)
	{
		const char* p = this->to_string();

		if (p == nullptr) p = "<corrupted event>";

		fwrite(p, sizeof(*p), strlen(p), f);
		if (this->type == event_type_t::mouse_move)
		{
			this->mouse_move_data;
			printf(": %i %i\n", this->mouse_move_data.x, this->mouse_move_data.x);
		}
		else if (this->type == event_type_t::mouse_press || this->type == event_type_t::mouse_release)
		{
			const char* button_name = [](mouse_button_t button) -> const char*
			{
				switch (button)
				{
				case mouse_button_t::left: return "left";
				case mouse_button_t::right: return "right";
				case mouse_button_t::middle: return "middle";
				case mouse_button_t::extra1: return "extra1";
				case mouse_button_t::extra2: return "extra2";
				default: return "<corrupted data>";
				}
			}(this->mouse_button_data.button);

			fwrite(": ", sizeof(char), 2, stdout);
			fwrite(button_name, sizeof(*button_name), strlen(button_name), stdout);
			printf(" at %i %i\n", (int)this->mouse_button_data.x, (int)this->mouse_button_data.y);
		}
		else if (this->type == event_type_t::mouse_scroll)
		{
			const char* button_name = this->mouse_scroll_data.is_vertical ? "vertical" : "hotizontal";
			fwrite(": ", sizeof(char), 2, stdout);
			fwrite(button_name, sizeof(*button_name), strlen(button_name), stdout);
			printf(" by %g\n", this->mouse_scroll_data.delta);
		}
		else if (this->type == event_type_t::create || this->type == event_type_t::close)
		{
			putchar('\n');
		}
		else
		{
			fwrite(": Dump not implemented\n", sizeof(char), 23, stdout);
		}
	}*/
};



using window_open_result_t = uint8_t;
struct window_open_result
{
	//All fine
	static constexpr window_open_result_t ok = 0;

	//Not (possibly yet) implemented by the library
	static constexpr window_open_result_t unimplemented = 1;

	//Error after calling system API
	//(except for window creation function itself)
	static constexpr window_open_result_t system_error = 2;

	//Error after calling OpenGL API
	static constexpr window_open_result_t opengl_error = 3;

	//Window creation system function failed
	static constexpr window_open_result_t window_creation_error = 4;

	//Invalid window size (too big/small)
	static constexpr window_open_result_t window_size_error = 5;

	//Failed to initialize GLEW
	static constexpr window_open_result_t glew_error = 6;

	//OpenGL feature required is not supported
	static constexpr window_open_result_t opengl_unsupported_function = 7;

	//Everything went fine except that window is not ready for direct drawing with window_t::draw_pixels_*
	static constexpr window_open_result_t ok_but_direct_drawing_unsupported = 8;

};



using window_style_t = uint16_t;
struct window_style
{
	static constexpr window_style_t border = 1;
	static constexpr window_style_t close_button = 2;
	static constexpr window_style_t min_button = 4;
	static constexpr window_style_t resize = 8;
	static constexpr window_style_t caption = 16;
	static constexpr window_style_t fullscreen = 32;
	static constexpr window_style_t max_button = 64;
	static constexpr window_style_t hidden = 128;

	static constexpr window_style_t close_min_max = min_button | max_button | close_button;

	static constexpr window_style_t default_style = border | close_min_max | resize | caption;
};



class window_t
{
public:
	
	class _window_impl;
	class _window_independend_impl; //OS-independend stuff

#if defined _KSN_COMPILER_MSVC && defined _WIN32

	using native_window_t = HWND__*;
	using native_event_t = tagMSG;

#ifdef _WIN64
	ksn::fast_pimpl<_window_impl, 88, 8> m_impl;
#else
	ksn::fast_pimpl<_window_impl, 52, 4> m_impl;
#endif
#else

#error This platform is not (possibly yet) supported by <ksn/window.hpp>

#endif

	ksn::fast_pimpl<_window_independend_impl, sizeof(uint64_t) + sizeof(uint64_t), alignof(uint64_t)> m_impl_indep;



public:





	native_window_t window_native_handle() const noexcept;

	window_t() noexcept;
	window_t(const window_t&) = delete;
	window_t(window_t&&) noexcept;
	~window_t() noexcept;

	window_t(uint16_t width, uint16_t height, const char* title = "", window_style_t style = window_style::default_style) noexcept;
	window_t(uint16_t width, uint16_t height, const wchar_t* title, window_style_t style = window_style::default_style) noexcept;

	virtual window_open_result_t open(uint16_t width, uint16_t height, const char* title = "", window_style_t style = window_style::default_style) noexcept;
	virtual window_open_result_t open(uint16_t width, uint16_t height, const wchar_t* title, window_style_t style = window_style::default_style) noexcept;

	virtual void close() noexcept;

	bool poll_event(event_t&) noexcept;
	bool wait_event(event_t&) noexcept;

	void discard_all_events() noexcept;
	void discard_stored_events() noexcept;

	bool is_open() const noexcept;

	bool has_focus() const noexcept;
	void request_focus() const noexcept;

	//Limits the frame rate (use window_t::tick() to wait for a frame end)
	//0 = unlimited framerate
	//Unlimited by default
	void set_framerate(uint32_t fps) noexcept;
	uint32_t get_framerate() const noexcept;

	//Refresh rate of the monitor the OS considers window to be on
	uint32_t get_monitor_framerate() const noexcept;

	//Updates internal time counter and waits for a frame end according to set framerate
	//is a no-op if framerate is unlimited (that is, 0)
	void tick() noexcept;

	uint16_t get_width() const noexcept;
	uint16_t get_height() const noexcept;
	std::pair<uint16_t, uint16_t> get_size() const noexcept;

	void hide() const noexcept;
	void show() const noexcept;


#ifdef _WIN32
	HDC__* winapi_get_hdc() const noexcept;
#endif

	//Draw a rectangle of pixels in BGR format (3 bytes per pixel) right into the screen front buffer at position (x; y) ((0; 0) is top left)
	//Size dimension of -1 means the whole window size
	void draw_pixels_bgr_front(const void*, uint16_t x = 0, uint16_t y = 0, uint16_t width = -1, uint16_t height = -1);

	//Draw a rectangle of pixels in BGRA format (4 bytes per pixel) right into the screen front buffer at position (x; y) ((0; 0) is top left)
	//Size dimension of -1 means the whole window size
	//Alpha channel is discarded
	void draw_pixels_bgra_front(const void*, uint16_t x = 0, uint16_t y = 0, uint16_t width = -1, uint16_t height = -1);

};



_KSN_END



#endif //!_KSN_WINDOW_HPP_
