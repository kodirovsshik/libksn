

#include <ksn/window.hpp>

#ifdef _KSN_COMPILER_MSVC

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")

#include <Windows.h>

#endif

int main()
{
	std::pair<uint16_t, uint16_t> size{ 800, 600 };
	std::pair<int16_t, int16_t> pos;
	bool is_fullscreen = false;
	ksn::window_t win(size.first, size.second);
	pos = win.get_client_position();

	while (win.is_open())
	{




		ksn::event_t ev;

		while (win.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				win.close();
				break;

			case ksn::event_type_t::resize:
				if (!is_fullscreen)
					size = { ev.window_resize_data.width_new, ev.window_resize_data.height_new };
				break;
				
			case ksn::event_type_t::move:
				if (!is_fullscreen)
					pos = { ev.window_move_data.x_new, ev.window_move_data.y_new };
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					win.close();
					break;

				case ksn::keyboard_button_t::tab:
					is_fullscreen ^= 1;
					if (!is_fullscreen)
					{
						win.set_client_position(pos);
						win.set_client_size(size);
					}
					else
					{
						win.set_fullscreen_windowed();
					}
					win.set_cursor_capture(is_fullscreen);
					win.set_cursor_visible(!is_fullscreen);
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}
	}
}
