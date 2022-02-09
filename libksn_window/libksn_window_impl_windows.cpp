
#include <ksn/window.hpp>
#include <ksn/unicode.hpp>

#include <Windows.h>

#include <stdio.h>

#include <queue>
#include <mutex>


_KSN_BEGIN


namespace
{
	static BOOL AdjustWindowRectInverse(LPRECT rect, DWORD dwStyle)
	{
		RECT rc;
		SetRectEmpty(&rc);
		bool ok = AdjustWindowRect(&rc, dwStyle, false);
		if (ok)
		{
			rect->left -= rc.left;
			rect->right -= rc.right;
			rect->top -= rc.top;
			rect->bottom -= rc.bottom;
		}
		return ok;
	}

	static std::pair<uint16_t, uint16_t> adjust_size_to_client(int width, int height, HWND window)
	{
		RECT rc{};
		rc.right = width;
		rc.bottom = height;
		AdjustWindowRectInverse(&rc, GetWindowLong(window, GWL_STYLE));
		return std::pair<uint16_t, uint16_t>
		{
			(uint16_t)std::min<long>(rc.right - rc.left, UINT16_MAX),
			(uint16_t)std::min<long>(rc.bottom - rc.top, UINT16_MAX)
		};
	}

	struct minmax_info
	{
		uint16_t width_min;
		uint16_t width_max;
		uint16_t height_min;
		uint16_t height_max;
	};

	static minmax_info GetSystemMetricsClient(HWND wnd)
	{
		minmax_info info;

		RECT rcmin{};
		rcmin.right = GetSystemMetrics(SM_CXMINTRACK);
		rcmin.bottom = GetSystemMetrics(SM_CYMINTRACK);

		RECT rcmax{};
		rcmax.right = GetSystemMetrics(SM_CXMAXTRACK);
		rcmax.bottom = GetSystemMetrics(SM_CYMAXTRACK);

		DWORD style = GetWindowLongA(wnd, GWL_STYLE);

		AdjustWindowRectInverse(&rcmin, style);
		AdjustWindowRectInverse(&rcmax, style);

		info.width_min = (uint16_t)std::min<long>(rcmin.right - rcmin.left, UINT16_MAX);
		info.width_max = (uint16_t)std::min<long>(rcmax.right - rcmax.left, UINT16_MAX);
		info.height_min = (uint16_t)std::min<long>(rcmin.bottom - rcmin.top, UINT16_MAX);
		info.height_max = (uint16_t)std::min<long>(rcmax.bottom - rcmax.top, UINT16_MAX);

		return info;
	}

}

class window_t::_window_impl
{

#define lock_event_queue(impl) \
		auto __event_queue_lock = (impl)->m_is_thread_safe_events ? \
			std::unique_lock<std::mutex>((impl)->m_queue_mutex) \
			: std::unique_lock<std::mutex>();


	friend class window_t;



public:

	void ensure_size_constraint()
	{
		auto size = this->m_ksn_window->get_client_size();

		std::pair<uint16_t, uint16_t> new_size =
		{
			std::clamp(size.first, this->m_size_min.first, this->m_size_max.first),
			std::clamp(size.second, this->m_size_min.second, this->m_size_max.second)
		};

		this->m_ksn_window->set_client_size(new_size);
	}

	static LRESULT WINAPI __ksn_wnd_proc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
	{
		msg = LOWORD(msg);



		if (msg == WM_CREATE)
		{
			ksn::window_t::_window_impl& win_impl = *(window_t::_window_impl*)(((CREATESTRUCTW*)(l))->lpCreateParams);
			std::deque<event_t>& q = win_impl.m_queue;

			lock_event_queue(&win_impl);

			SetWindowLongPtrW(wnd, GWLP_USERDATA, (LONG_PTR)&win_impl);

			event_t ev;
			ev.type = event_type_t::create;
			q.push_back(ev);

			return 0;
		}

		else if (msg == WM_GETMINMAXINFO)
		{
			MINMAXINFO* info = (MINMAXINFO*)l;

			ksn::window_t::_window_impl* pimpl = (window_t::_window_impl*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
			if (!pimpl)
			{
				RECT rc{};
				rc.right = rc.bottom = 65535;

				AdjustWindowRect(&rc, GetWindowLongA(wnd, GWL_STYLE), false);
				info->ptMaxTrackSize.x = rc.right - rc.left;
				info->ptMaxTrackSize.y = rc.bottom - rc.top;
				return 0;
			}

			ksn::window_t::_window_impl& win_impl = *pimpl;

			DWORD style = GetWindowLongA(win_impl.m_window, GWL_STYLE);

			RECT rcmin{}, rcmax{};
			rcmin.right = win_impl.m_size_min.first;
			rcmin.bottom = win_impl.m_size_min.second;
			rcmax.right = win_impl.m_size_max.first;
			rcmax.bottom = win_impl.m_size_max.second;

			AdjustWindowRect(&rcmin, style, false);
			AdjustWindowRect(&rcmax, style, false);

			info->ptMinTrackSize.x = rcmin.right - rcmin.left;;
			info->ptMinTrackSize.y = rcmin.bottom - rcmin.top;
			info->ptMaxTrackSize.x = rcmax.right - rcmax.left;
			info->ptMaxTrackSize.y = rcmax.bottom - rcmax.top;
			return 0;
		}


		ksn::window_t::_window_impl& win_impl = *(window_t::_window_impl*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
		std::deque<event_t>& q = win_impl.m_queue;


#define push_mouse_event(_button, is_pressed) \
		{\
			event_t ev; \
			ev.type = is_pressed ? event_type_t::mouse_press : event_type_t::mouse_release; \
			ev.mouse_button_data.button = mouse_button_t::_button; \
			ev.mouse_button_data.x = (uint16_t)LOWORD(l); \
			ev.mouse_button_data.y = (uint16_t)HIWORD(l); \
			lock_event_queue(&win_impl); \
			q.push_back(ev); \
		}\



#define push_mouse_scroll_event(vertical) \
		{ \
			POINT pos{ (LONG)LOWORD(l), (LONG)HIWORD(l) }; \
			ScreenToClient(win_impl.m_window, &pos); \
 \
			event_t ev; \
			ev.type = vertical ? event_type_t::mouse_scroll_vertical : event_type_t::mouse_scroll_horizontal; \
			ev.mouse_scroll_data.is_vertical = (msg == WM_MOUSEWHEEL); \
			ev.mouse_scroll_data.delta = (float)(int16_t)HIWORD(w) / 120; \
			ev.mouse_scroll_data.x = (uint16_t)pos.x; \
			ev.mouse_scroll_data.y = (uint16_t)pos.y; \
 \
			lock_event_queue(&win_impl);\
			q.push_back(ev); \
		}


		switch (msg)
		{
		case WM_CLOSE:
		{
			event_t ev;
			ev.type = event_type_t::close;

			lock_event_queue(&win_impl);
			q.push_back(ev);
			return 0;
		}
		break;

		case WM_SIZE:
		{
			if (w == SIZE_MAXIMIZED)
			{
				event_t ev;
				ev.type = event_type_t::maximized;
				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
			
			if (w == SIZE_MINIMIZED)
			{
				event_t ev;
				ev.type = event_type_t::minimized;
				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
			else if (w != SIZE_MAXHIDE && w != SIZE_MAXSHOW)
			{
				resizemove_data_t data;
				data.window = win_impl.m_ksn_window;

				auto new_size = win_impl.m_ksn_window->get_client_size();

				if (new_size == win_impl.m_resizemove_last_size)
					break;

				data.resize = true;
				data.window_resize_data.width_old = win_impl.m_resizemove_last_size.first;
				data.window_resize_data.height_old = win_impl.m_resizemove_last_size.second;
				data.window_resize_data.width_new = new_size.first;
				data.window_resize_data.height_new = new_size.second;

				DeleteObject(win_impl.m_bitmap);
				win_impl.m_bitmap = CreateCompatibleBitmap(win_impl.m_hdc, new_size.first, new_size.second);
				SelectObject(win_impl.m_hmdc, win_impl.m_bitmap);

				win_impl.m_resizemove_last_size = new_size;
				if (win_impl.m_resizemove_handle)
					win_impl.m_resizemove_handle(&data);
				else
				{
					event_t ev;
					ev.type = event_type_t::resize;

					ev.window_resize_data.height_new = data.window_resize_data.height_new;
					ev.window_resize_data.height_old = data.window_resize_data.height_old;
					ev.window_resize_data.width_new = data.window_resize_data.width_new;
					ev.window_resize_data.width_old = data.window_resize_data.width_old;

					lock_event_queue(&win_impl);
					q.push_back(ev);
				}
			}
		}
		break;

		case WM_MOVE:
		{
			resizemove_data_t data;
			data.window = win_impl.m_ksn_window;

			auto new_pos = win_impl.m_ksn_window->get_client_position();

			data.move = true;
			data.window_move_data.x_old = win_impl.m_resizemove_last_pos.first;
			data.window_move_data.y_old = win_impl.m_resizemove_last_pos.second;
			data.window_resize_data.width_new = new_pos.first;
			data.window_resize_data.height_new = new_pos.second;

			win_impl.m_resizemove_last_pos = new_pos;
			if (win_impl.m_resizemove_handle)
				win_impl.m_resizemove_handle(&data);
			else
			{
				event_t ev;
				ev.type = event_type_t::move;

				ev.window_move_data.x_new = data.window_move_data.x_new;
				ev.window_move_data.x_old = data.window_move_data.x_old;
				ev.window_move_data.y_new = data.window_move_data.y_new;
				ev.window_move_data.y_old = data.window_move_data.y_old;

				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
		}
		break;

		case WM_ENTERSIZEMOVE:
		{
			ClipCursor(NULL);

			RECT client_area;
			GetClientRect(win_impl.m_window, &client_area);

			win_impl.m_last_pos.first = (int32_t)client_area.left;
			win_impl.m_last_pos.second = (int32_t)client_area.bottom;

			win_impl.m_last_size.first = (uint16_t)(client_area.right - client_area.left);
			win_impl.m_last_size.second = (uint16_t)(client_area.bottom - client_area.top);

			win_impl.m_is_resizemove = true;
		}
		break;

		case WM_EXITSIZEMOVE:
		{
			WINDOWINFO info;
			info.cbSize = sizeof(info);
			GetWindowInfo(win_impl.m_window, &info);
			RECT client_area = info.rcClient;;

			if (win_impl.m_is_clipping)
				ClipCursor(&client_area);

			uint16_t width = uint16_t(client_area.right - client_area.left);
			uint16_t height = uint16_t(client_area.bottom - client_area.top);

			auto new_size = std::pair<uint16_t, uint16_t>(width, height);
			auto new_pos = std::pair<int32_t, int32_t>((int32_t)client_area.left, (int32_t)client_area.top);

			win_impl.m_is_resizemove = false;

			if (new_pos != win_impl.m_last_pos)
			{
				event_t ev;
				ev.type = event_type_t::move;

				ev.window_move_data.x_new = new_pos.first;
				ev.window_move_data.y_new = new_pos.second;

				ev.window_move_data.x_old = win_impl.m_last_pos.first;
				ev.window_move_data.y_old = win_impl.m_last_pos.second;

				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
			if (new_size != win_impl.m_last_size)
			{
				DeleteObject(win_impl.m_bitmap);
				win_impl.m_bitmap = CreateCompatibleBitmap(win_impl.m_hdc, new_size.first, new_size.second);
				SelectObject(win_impl.m_hmdc, win_impl.m_bitmap);

				event_t ev;
				ev.type = event_type_t::resize;

				ev.window_resize_data.width_new = new_size.first;
				ev.window_resize_data.height_new = new_size.second;

				ev.window_resize_data.width_old = win_impl.m_last_size.first;
				ev.window_resize_data.height_old = win_impl.m_last_size.second;

				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
		}
		break;

		case WM_SETFOCUS:
		{
			if (win_impl.m_is_clipping)
			{
				WINDOWINFO info;
				info.cbSize = sizeof(info);
				GetWindowInfo(win_impl.m_window, &info);
				ClipCursor(&info.rcClient);
			}

			event_t ev;
			ev.type = event_type_t::focus_gained;
			lock_event_queue(&win_impl);
			q.push_back(ev);
		}
		break;

		case WM_KILLFOCUS:
		{
			ClipCursor(NULL);

			if (!win_impl.m_is_closing)
			{
				event_t ev;
				ev.type = event_type_t::focus_lost;
				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
		}
		break;

		case WM_CHAR:
		{
			if (win_impl.m_is_repetitive_keyboard_enabled || (l & 0xFFFF) == 0)
			{
				wchar_t ch = (wchar_t)w;

				uint8_t high6 = ch >> 10; //6 bits of header
				wchar_t high_ch = win_impl.m_pending_wchar; //previous UTF-16 part of character (if present)

				if (high6 == 0b110110)
				{
					win_impl.m_pending_wchar = ch;
				}
				else if (high6 == 0b110111 && high_ch)
				{
					event_t ev;
					ev.type = event_type_t::text;
					ev.character = ((high_ch & 0b0000001111111111) << 16) | (ch & 0b0000001111111111);

					lock_event_queue(&win_impl);
					q.push_back(ev);

					win_impl.m_pending_wchar = 0;
				}
				else //Single UTF-16 unit
				{
					event_t ev;
					ev.type = event_type_t::text;
					ev.character = (uint32_t)w;

					lock_event_queue(&win_impl);
					q.push_back(ev);
				}
			}
		}
		break;

		case WM_UNICHAR:
		{
			if (w == UNICODE_NOCHAR) return FALSE; //As microsoft intended

			if (win_impl.m_is_repetitive_keyboard_enabled || (l & 0xFFFF) == 0)
			{
				event_t ev;
				ev.type = event_type_t::text;
				ev.character = (uint32_t)w;
			}
		}
		break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			bool press = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
			bool repeated = HIWORD(l) & KF_REPEAT;
			if (!press || win_impl.m_is_repetitive_keyboard_enabled || !repeated)
			{
				event_t ev;
				if (press)
				{
					if (repeated)
						ev.type = event_type_t::keyboard_hold;
					else
						ev.type = event_type_t::keyboard_press;
				}
				else 
					ev.type = event_type_t::keyboard_release;

				ev.keyboard_button_data.button = my_t::_get_button(w, l);

				if (win_impl.m_check_special_keys_on_keyboard_event)
				{
					//TODO: test this code
					ev.keyboard_button_data.alt = HIWORD(GetKeyState(VK_MENU)) != 0;
					ev.keyboard_button_data.control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
					ev.keyboard_button_data.shift = HIWORD(GetKeyState(VK_SHIFT)) != 0;
					ev.keyboard_button_data.system = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
				}

				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
		}
		break;

		case WM_MOUSEWHEEL:
			push_mouse_scroll_event(true);
			break;

		case WM_MOUSEHWHEEL:
			push_mouse_scroll_event(false);
			break;

		case WM_LBUTTONDOWN:
			push_mouse_event(left, true);
			break;

		case WM_LBUTTONUP:
			push_mouse_event(left, false);
			break;

		case WM_RBUTTONDOWN:
			push_mouse_event(right, true);
			break;

		case WM_RBUTTONUP:
			push_mouse_event(right, false);
			break;

		case WM_MBUTTONDOWN:
			push_mouse_event(middle, true);
			break;

		case WM_MBUTTONUP:
			push_mouse_event(middle, false);
			break;

		case WM_XBUTTONUP:
			if (HIWORD(w) == XBUTTON1)
				push_mouse_event(extra1, true)
			else
				push_mouse_event(extra2, true);
			break;

		case WM_XBUTTONDOWN:
			if (HIWORD(w) == XBUTTON1)
				push_mouse_event(extra1, false)
			else
				push_mouse_event(extra2, false);
			break;

		case WM_MOUSELEAVE:
		{
			if (win_impl.m_mouse_inside)
			{
				win_impl.m_mouse_inside = false;

				ksn::event_t ev;
				ev.type = event_type_t::mouse_leave;

				lock_event_queue(&win_impl);
				q.push_back(ev);
			}
		}
		break;

		case WM_MOUSEMOVE:
		{
			event_t ev;

			if (win_impl.m_mouse_inside == false)
			{
				ev.type = event_type_t::mouse_entered;
				{
					lock_event_queue(&win_impl);
					q.push_back(ev);
				}
				win_impl.m_mouse_inside = true;

				TRACKMOUSEEVENT track_info;
				track_info.cbSize = sizeof(track_info);
				track_info.dwFlags = TME_LEAVE;
				track_info.hwndTrack = win_impl.m_window;
				TrackMouseEvent(&track_info);
			}
			int32_t x = (int32_t)LOWORD(l);
			int32_t y = (int32_t)HIWORD(l);

			ev.type = event_type_t::mouse_move;
			ev.mouse_move_data.x = x;
			ev.mouse_move_data.y = y;

			lock_event_queue(&win_impl);
			q.push_back(ev);
		}
		break;

#undef push_mouse_event
#undef push_mouse_scroll_event
		//case WM_SHOWWINDOW:
		case WM_PAINT:
			win_impl.check_initial_blackout();
		break;

		default:
			break;
		};


		if ((msg == WM_SYSCOMMAND) && (w == SC_KEYMENU))
			return 0;

		return DefWindowProcW(wnd, msg, w, l);
	}



	using my_t = window_t::_window_impl;


public:

	std::deque<event_t> m_queue;
	std::mutex m_queue_mutex;

	HWND m_window = nullptr;
	HDC m_hdc = nullptr;
	HDC m_hmdc = nullptr; //memory device context //P.S. i feel like this OS will never stop surprising me
	HBITMAP m_bitmap = nullptr;

	window_t* m_ksn_window = nullptr;

	window_resizemove_handle_t m_resizemove_handle = nullptr;

	std::pair<uint16_t, uint16_t> m_last_size{};
	std::pair<uint16_t, uint16_t> m_resizemove_last_size{};

	std::pair<int32_t, int32_t> m_last_pos{};
	std::pair<int32_t, int32_t> m_resizemove_last_pos{};

	std::pair<uint16_t, uint16_t> m_size_min{};
	std::pair<uint16_t, uint16_t> m_size_max = { UINT16_MAX, UINT16_MAX };

	wchar_t m_pending_wchar = 0;

	union
	{
		struct
		{
			bool m_is_repetitive_keyboard_enabled : 1;
			bool m_is_resizemove : 1; //Whether WM_ENTERSIZEMOVE was received but WM_EXITSIZEMOVE hasn't been yet
			bool m_check_special_keys_on_keyboard_event : 1;
			bool m_mouse_inside : 1;
			bool m_is_closing : 1;
			bool m_is_clipping : 1;
			bool m_is_thread_safe_events : 1;
			mutable bool m_filled_on_init : 1;
		};
		uint16_t m_flags;
	};


private:

	static bool _process_pfd(HDC hdc)
	{
		//Do boring repetitive WINAPI stuff 
		PIXELFORMATDESCRIPTOR pfd{};
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;

		return SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
	}

	static void _process_msg(MSG& msg)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	static keyboard_button_t _get_button(WPARAM key, LPARAM flags)
	{
		using kb = keyboard_button_t;
		switch (key)
		{
		case VK_SHIFT:
		{
			static const UINT lshift_scancode = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
			UINT current_scancode = (flags >> 16) & 0xFF;
			return current_scancode == lshift_scancode ? kb::shift_left : kb::shift_right;
		}
		case VK_MENU: return (HIWORD(flags) & KF_EXTENDED) ? kb::alt_right : kb::alt_left;
		case VK_CONTROL: return (HIWORD(flags) & KF_EXTENDED) ? kb::ctrl_right : kb::ctrl_left;
		case VK_LWIN: return kb::system_left;
		case VK_RWIN: return kb::system_right;
		case VK_APPS: return (HIWORD(flags) & KF_EXTENDED) ? kb::menu_right : kb::menu_left;
		case VK_OEM_1: return kb::semicolon;
		case VK_OEM_PLUS: return kb::equal_sign;
		case VK_OEM_COMMA: return kb::comma;
		case VK_OEM_MINUS: return kb::minus;
		case VK_OEM_PERIOD: return kb::period;
		case VK_OEM_2: return kb::slash;
		case VK_OEM_3: return kb::tilde;
		case VK_OEM_4: return kb::bracket_left;
		case VK_OEM_6: return kb::bracket_right;
		case VK_OEM_7: return kb::quote;
		case VK_OEM_5: return kb::backslash;
		case VK_ESCAPE: return kb::esc;
		case VK_SPACE: return kb::space;
		case VK_RETURN: return kb::enter;
		case VK_BACK: return kb::backspace;
		case VK_TAB: return kb::tab;
		case VK_PRIOR: return kb::page_up;
		case VK_NEXT: return kb::page_down;
		case VK_END: return kb::end;
		case VK_HOME: return kb::home;
		case VK_INSERT: return kb::insert;
		case VK_DELETE: return kb::delete_;
		case VK_ADD: return kb::add;
		case VK_SUBTRACT: return kb::substract;
		case VK_MULTIPLY: return kb::multiply;
		case VK_DIVIDE: return kb::divide;
		case VK_PAUSE: return kb::pause;
		case VK_F1: return kb::F1;
		case VK_F2: return kb::F2;
		case VK_F3: return kb::F3;
		case VK_F4: return kb::F4;
		case VK_F5: return kb::F5;
		case VK_F6: return kb::F6;
		case VK_F7: return kb::F7;
		case VK_F8: return kb::F8;
		case VK_F9: return kb::F9;
		case VK_F10: return kb::F10;
		case VK_F11: return kb::F11;
		case VK_F12: return kb::F12;
		case VK_F13: return kb::F13;
		case VK_F14: return kb::F14;
		case VK_F15: return kb::F15;
		case VK_F16: return kb::F16;
		case VK_F17: return kb::F17;
		case VK_F18: return kb::F18;
		case VK_F19: return kb::F19;
		case VK_F20: return kb::F20;
		case VK_F21: return kb::F21;
		case VK_F22: return kb::F22;
		case VK_F23: return kb::F23;
		case VK_F24: return kb::F24;
		case VK_LEFT: return kb::arrow_left;
		case VK_RIGHT: return kb::arrow_right;
		case VK_UP: return kb::arrow_up;
		case VK_DOWN: return kb::arrow_down;

		case VK_NUMPAD0:
		case VK_NUMPAD1:
		case VK_NUMPAD2:
		case VK_NUMPAD3:
		case VK_NUMPAD4:
		case VK_NUMPAD5:
		case VK_NUMPAD6:
		case VK_NUMPAD7:
		case VK_NUMPAD8:
		case VK_NUMPAD9:
			return keyboard_button_t((int)kb::numpad0 + (int)key - VK_NUMPAD0);

		case VK_NUMLOCK: return kb::num_lock;
		case VK_CAPITAL: return kb::caps_lock;
		case VK_SCROLL: return kb::scroll_lock;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			return keyboard_button_t((int)kb::a + (int)key - 'A');

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return keyboard_button_t((int)kb::digit0 + (int)key - '0');

		default:
			return kb::other;
		}
	}

	void init_flags()
	{
		this->m_is_repetitive_keyboard_enabled = false;
		this->m_is_resizemove = false;
		this->m_check_special_keys_on_keyboard_event = true;
		this->m_mouse_inside = false;
		this->m_is_closing = false;
		this->m_is_clipping = false;
		this->m_is_thread_safe_events = true;
		this->m_filled_on_init = false;
	}



public:

	_window_impl(const _window_impl&) = delete;

	_window_impl() noexcept
	{
		this->init_flags();
	}
	_window_impl(_window_impl&& rhs) noexcept
	{
		this->_window_impl::_window_impl();
		*this = std::move(rhs);
	}
	~_window_impl() noexcept
	{
		this->close();
	}

	_window_impl& operator=(_window_impl&& rhs) noexcept
	{
		if (&static_cast<const _window_impl&>(rhs) == this)
			return *this;

		this->close();
		this->swap(std::move(rhs));

		return *this;
	}

	template<class ref_t>
	void swap(ref_t&& rhs)
	{
		//Well...
		std::swap(this->m_queue, rhs.m_queue);
		std::swap(this->m_window, rhs.m_window);
		std::swap(this->m_hdc, rhs.m_hdc);
		std::swap(this->m_hmdc, rhs.m_hmdc);
		std::swap(this->m_bitmap, rhs.m_bitmap);
		std::swap(this->m_ksn_window, rhs.m_ksn_window);
		std::swap(this->m_resizemove_handle, rhs.m_resizemove_handle);
		std::swap(this->m_last_size, rhs.m_last_size);
		std::swap(this->m_resizemove_last_size, rhs.m_resizemove_last_size);
		std::swap(this->m_last_pos, rhs.m_last_pos);
		std::swap(this->m_resizemove_last_pos, rhs.m_resizemove_last_pos);
		std::swap(this->m_size_min, rhs.m_size_min);
		std::swap(this->m_size_max, rhs.m_size_max);
		std::swap(this->m_pending_wchar, rhs.m_pending_wchar);
		std::swap(this->m_flags, rhs.m_flags);

		//We don't care about the queues' mutexes cuz if you move your window to another object
		//in one thread and are still using it in another thread then you are probably doing something wrong
	}

	void close() noexcept
	{
		this->m_ksn_window = nullptr;

		this->m_is_closing = true;
		if (this->m_hdc)
		{
			ReleaseDC(this->m_window, this->m_hdc);
			DeleteDC(this->m_hmdc);
			DeleteObject(this->m_bitmap);
			this->m_hdc = nullptr;
			this->m_hmdc = nullptr;
			this->m_bitmap = nullptr;
		}
		if (this->m_window)
		{
			DestroyWindow(this->m_window);
			this->m_window = nullptr;
			this->m_queue.clear();
		}
		this->m_is_closing = false;
	}


	template<typename char_t>
	static std::wstring to_wchar(const char_t* str)
	{
		if constexpr (std::is_same_v<char_t, wchar_t>)
			return std::wstring(str);

		return ksn::unicode_string_convert<std::wstring>(std::basic_string<char_t>(str));
	}

	template<typename char_t>
	window_open_result_t open(uint16_t width, uint16_t height, const char_t* window_name, window_style_t window_style, window_t* ksn_win) noexcept
	{
		//Just a wrapper function for "real" _Xopen

		this->close();

		this->m_ksn_window = ksn_win;
		window_open_result_t result = this->_Xopen(width, height, to_wchar(window_name).data(), window_style);

		if (result == window_open_result::ok || result == window_open_result::ok_but_direct_drawing_unsupported)
		{
			WINDOWINFO window_info;
			window_info.cbSize = sizeof(window_info);
			GetWindowInfo(this->m_window, &window_info);

			//Check whether mouse is initially inside the window
			if constexpr (true)
			{
				POINT cursor_pos;
				GetCursorPos(&cursor_pos);
				ScreenToClient(this->m_window, &cursor_pos);

				this->m_size_min = adjust_size_to_client(GetSystemMetrics(SM_CXMINTRACK), GetSystemMetrics(SM_CYMINTRACK), this->m_window);

				this->m_mouse_inside =
					cursor_pos.x >= window_info.rcClient.left &&
					cursor_pos.x < window_info.rcClient.right&&
					cursor_pos.y >= window_info.rcClient.top &&
					cursor_pos.y < window_info.rcClient.bottom;

				this->m_last_size.first = uint16_t(window_info.rcClient.right - window_info.rcClient.left);
				this->m_last_size.second = uint16_t(window_info.rcClient.bottom - window_info.rcClient.top);

				this->m_last_pos.first = window_info.rcClient.left;
				this->m_last_pos.second = window_info.rcClient.top;

				this->m_resizemove_last_pos = this->m_last_pos;
				this->m_resizemove_last_size = this->m_last_size;
			}

			//Set mouse tracking for WM_MOUSELEAVE
			if constexpr (true)
			{
				TRACKMOUSEEVENT track_info;
				track_info.cbSize = sizeof(track_info);
				track_info.dwFlags = TME_LEAVE;
				track_info.hwndTrack = this->m_window;
				TrackMouseEvent(&track_info);
			}

			//Set cursor to be an arrow and not the whatever-comes-to-os's-mind
			SetCursor(LoadCursorW(nullptr, IDC_ARROW));

			//Window is created hidden initially
			if (!(window_style & window_style::hidden)) ShowWindow(this->m_window, SW_SHOW);

			//Get rid of all "default" messages
			if constexpr (true)
			{
				MSG msg;
				int threshold = 32; //But don't keep going for too long
				while (threshold-- > 0)
				{
					if (PeekMessageW(&msg, this->m_window, 0, 0, PM_REMOVE) <= 0)
					{
						break;
					}
					
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
				_KSN_DEBUG_EXPR(if (threshold < 0) printf("WINDOW DEFAULT MESSAGES THRESHOLD REACHED\n\a"));
				//^^^ Just in case ^^^
			}
		}
		else
		{
			//If something has broken, undid everything that hasn't but preserve the WinAPI error
			int last_error = GetLastError();
			this->close();
			SetLastError(last_error);
		}
		return result;
	}

	window_open_result_t _Xopen(uint16_t width, uint16_t height, const wchar_t* window_name, window_style_t window_style) noexcept
	{
		if (window_style & window_style::fullscreen) return window_open_result::unimplemented; //TODO


		constexpr static UINT winapi_flags[] =
		{
			WS_BORDER, //Border = 1, WS_BORDER
			WS_SYSMENU | WS_CAPTION, //Close button = 2, WS_SYSMENU.
			WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, //Minimize button = 4, WS_MINIMIZEBOX
			WS_THICKFRAME, //Resize = 8, WS_THICKFRAME
			WS_CAPTION, //Caption = 16, WS_CAPTION
			0, //Fullscreen = 32
			WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX, //Maximize button = 64, WS_MAXIMIZEBOX
			0
		};

		DWORD winapi_window_style = 0;
		for (int i = 0; i < 8; ++i) if (window_style & (1 << i)) winapi_window_style |= winapi_flags[i];

		RECT rect{ 0, 0, (LONG)width, (LONG)height };
		if (window_style & ~window_style::fullscreen) //if not fullscreen
		{
			//Adjust window size so that client area is exactly WxH
			AdjustWindowRectEx(&rect, winapi_window_style, FALSE, 0);
			width = uint16_t(rect.right - rect.left);
			height = uint16_t(rect.bottom - rect.top);
		}


		this->m_window = CreateWindowW(L"_LIBKSN_window_", window_name, winapi_window_style, CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height, nullptr, nullptr, nullptr, this);
		if (this->m_window == nullptr) return window_open_result::window_creation_error;


		uint16_t client_width, client_height;
		uint16_t window_width, window_height;

		if constexpr (true)
		{
			WINDOWINFO result_info;
			result_info.cbSize = sizeof(result_info);
			GetWindowInfo(this->m_window, &result_info);

			client_width = (uint16_t)(result_info.rcClient.right - result_info.rcClient.left);
			client_height = (uint16_t)(result_info.rcClient.bottom - result_info.rcClient.top);

			window_width = (uint16_t)(result_info.rcWindow.right - result_info.rcWindow.left);
			window_height = (uint16_t)(result_info.rcWindow.bottom - result_info.rcWindow.top);
		}

		//If the client area size was (for example) 5x5, Windows may create a window that is much larger
		//If this happens, report size error
		if (width != window_width || height != window_height)
			return window_open_result::window_size_error;


		//Create device context
		this->m_hdc = GetDC(this->m_window);
		if (this->m_hdc == nullptr) return window_open_result::system_error;


		//Do DC pixel format window_style if necessary
		if ((window_style & (1 << (sizeof(window_style_t) * CHAR_BIT - 1))) == 0)
			if (!my_t::_process_pfd(this->m_hdc)) return window_open_result::system_error;


		//Create a memory DC and a bitmap to suppport direct color data drawing
		this->m_hmdc = CreateCompatibleDC(this->m_hdc);
		this->m_bitmap = CreateCompatibleBitmap(this->m_hdc, client_width, client_height);
		SelectObject(this->m_hmdc, this->m_bitmap);


		if (this->m_hmdc == nullptr || this->m_bitmap == nullptr)
		{
			if (this->m_hmdc) DeleteDC(this->m_hmdc);
			if (this->m_bitmap) DeleteObject(this->m_bitmap);
			return window_open_result::ok_but_direct_drawing_unsupported;
		}
		else return window_open_result::ok;
	}

	bool is_open() const noexcept
	{
		//TODO: find a more efficient way
		WINDOWINFO info_struct;
		info_struct.cbSize = sizeof(WINDOWINFO);
		return GetWindowInfo(this->m_window, &info_struct) == TRUE;
	}


	void draw_pixels(const void* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t bits)
	{
		if (width == uint16_t(-1)) width = this->m_last_size.first;
		if (height == uint16_t(-1)) height = this->m_last_size.second;

		BITMAPINFO bitmapinfo{};
		bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapinfo.bmiHeader.biWidth = (int)width;
		bitmapinfo.bmiHeader.biHeight = -(int)height;
		bitmapinfo.bmiHeader.biPlanes = 1;
		bitmapinfo.bmiHeader.biBitCount = bits;
		bitmapinfo.bmiHeader.biCompression = BI_RGB;

		//Quick time measure shows that this is slower than me waking up on a weekend
		//Jokes aside, 4-5x slower
		//StretchDIBits(this->m_hdc, x, y, width, height, x, y, width, height, data, &bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

		SetDIBits(this->m_hmdc, this->m_bitmap, y, height, data, &bitmapinfo, DIB_RGB_COLORS);
		BitBlt(this->m_hdc, x, y, width, height, this->m_hmdc, x, y, SRCCOPY);
	}

	template<class char_t>
	bool set_title(const char_t* s) const noexcept
	{
		std::wstring namae = to_wchar(s);
		if (namae.length() == 0 && *s != 0)
			return false;

		return SetWindowTextW(this->m_window, namae.c_str());
	}

	void check_initial_blackout() const noexcept
	{
		if (!this->m_filled_on_init)
		{
			PAINTSTRUCT pm;
			BeginPaint(this->m_window, &pm);
			GetClientRect(this->m_window, &pm.rcPaint);

			HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
			FillRect(this->m_hdc, &pm.rcPaint, black_brush);
			DeleteObject(black_brush);

			EndPaint(this->m_window, &pm);
			this->m_filled_on_init = true;
		}
	}
};





namespace
{
	struct __library_constructor_t
	{
		__library_constructor_t() noexcept
		{
			WNDCLASSW wcW{};
			wcW.lpszClassName = L"_LIBKSN_window_";
			wcW.lpfnWndProc = window_t::_window_impl::__ksn_wnd_proc;
			wcW.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
			RegisterClassW(&wcW);
		}

		~__library_constructor_t() noexcept
		{
			UnregisterClassA("_LIBKSN_window_", NULL);
		}

	} static __library_constructor;
}





window_t::native_window_t window_t::window_native_handle() const noexcept
{
	return this->m_impl->m_window;
}




window_t::window_t() noexcept
{
}
window_t::window_t(window_t&& w) noexcept
	: m_impl(std::move(w.m_impl))
{
}
window_t::~window_t() noexcept
{
	this->m_impl->close();
}


void window_t::swap(window_t& other) noexcept
{
	this->m_impl->swap(*other.m_impl);
}



window_open_result_t window_t::open(uint16_t width, uint16_t height, const char* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style, this);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const wchar_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style, this);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const char8_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style, this);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const char16_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style, this);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const char32_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style, this);
}


void window_t::close() noexcept
{
	this->m_impl->close();
}



bool window_t::poll_event(event_t& event) noexcept
{
	MSG native;
	while (PeekMessageW(&native, this->m_impl->m_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&native);
		DispatchMessageW(&native);
	}

	if (this->m_impl->m_queue.size() != 0)
	{
		lock_event_queue(this->m_impl);
		event = this->m_impl->m_queue.front();
		this->m_impl->m_queue.pop_front();
		return true;
	}
	return false;
}
bool window_t::wait_event(event_t& event) noexcept
{
	if (this->m_impl->m_queue.size() != 0)
	{
		lock_event_queue(this->m_impl);
		event = this->m_impl->m_queue.front();
		this->m_impl->m_queue.pop_front();
		return true;
	}

	while (this->m_impl->m_queue.size() == 0)
	{
		MSG native;
		if (GetMessageW(&native, this->m_impl->m_window, 0, 0) < 0) return false;
		TranslateMessage(&native);
		DispatchMessageW(&native);
	}
	return this->poll_event(event);
}

void window_t::discard_stored_events() noexcept
{
	lock_event_queue(this->m_impl);
	this->m_impl->m_queue.clear();
}
void window_t::discard_all_events() noexcept
{
	MSG native;
	while (PeekMessageW(&native, this->m_impl->m_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&native);
		DispatchMessageW(&native);
	}
	this->discard_stored_events();
}

bool window_t::is_open() const noexcept
{
	return this->m_impl->is_open();
}
window_t::operator bool() const noexcept
{
	return this->is_open();
}

std::pair<uint16_t, uint16_t> window_t::get_client_size() const noexcept
{
	WINDOWINFO wi;
	wi.cbSize = sizeof(wi);
	if (!GetWindowInfo(this->m_impl->m_window, &wi)) return { 0, 0 };
	return { uint16_t(wi.rcClient.right - wi.rcClient.left), uint16_t(wi.rcClient.bottom - wi.rcClient.top) };
}

void window_t::set_client_size(uint16_t width, uint16_t height) noexcept
{
	RECT size = { 0, 0, (long)width, (long)height };
	auto window = this->m_impl->m_window;
	auto prev_size = this->get_client_size();

	AdjustWindowRectEx(&size, GetWindowLongA(window, GWL_STYLE), false, 0);
	SetWindowPos(window, NULL, 0, 0, (uint16_t)(size.right - size.left), (uint16_t)(size.bottom - size.top), SWP_NOMOVE | SWP_NOZORDER);
	
	auto new_size = this->get_client_size();
	this->m_impl->m_last_size = new_size;
	this->m_impl->m_resizemove_last_size = new_size;

	ksn::event_t ev;
	ev.type = ksn::event_type_t::resize;
	ev.window_resize_data.width_new = new_size.first;
	ev.window_resize_data.height_new = new_size.second;
	ev.window_resize_data.width_old = prev_size.first;
	ev.window_resize_data.height_old = prev_size.second;
	try
	{
		lock_event_queue(this->m_impl);
		this->m_impl->m_queue.push_back(ev);
	}
	catch (...) {}
}

std::pair<int16_t, int16_t> window_t::get_client_position() const noexcept
{
	WINDOWINFO wi;
	wi.cbSize = sizeof(wi);
	if (!GetWindowInfo(this->m_impl->m_window, &wi)) return { 0, 0 };
	return { int16_t(wi.rcClient.left), int16_t(wi.rcClient.top) };
}

void window_t::set_client_position(int16_t x, int16_t y) noexcept
{
	RECT pos;
	auto window = this->m_impl->m_window;
	GetClientRect(window, &pos);

	pos.right += x - pos.left;
	pos.bottom += y - pos.top;
	pos.left = x;
	pos.top = y;

	auto prev_pos = this->get_client_position();

	AdjustWindowRectEx(&pos, GetWindowLongA(window, GWL_STYLE), false, 0);
	SetWindowPos(window, NULL, pos.left, pos.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	auto new_pos = this->get_client_position();
	this->m_impl->m_last_pos = new_pos;
	this->m_impl->m_resizemove_last_pos = new_pos;

	event_t ev;
	ev.type = event_type_t::move;
	ev.window_move_data.x_new = new_pos.first;
	ev.window_move_data.y_new = new_pos.second;
	ev.window_move_data.x_old = prev_pos.first;
	ev.window_move_data.y_old = prev_pos.second;
	try
	{
		lock_event_queue(this->m_impl);
		this->m_impl->m_queue.push_back(ev);
	}
	catch (...) {}
}

uint32_t window_t::get_monitor_framerate() const noexcept
{
	DEVMODEA devmode{};
	devmode.dmSize = sizeof(devmode);
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	return devmode.dmDisplayFrequency;
}

void window_t::hide() const noexcept
{
	ShowWindow(this->m_impl->m_window, SW_HIDE);
}
void window_t::show() const noexcept
{
	ShowWindow(this->m_impl->m_window, SW_SHOW);
	this->m_impl->check_initial_blackout();
}



HDC__* window_t::winapi_get_device_context() const noexcept
{
	return this->m_impl->m_hdc;
}
HDC__* window_t::winapi_get_memory_device_context() const noexcept
{
	return this->m_impl->m_hmdc;
}
HBITMAP__* window_t::winapi_get_bitmap() const noexcept
{
	return this->m_impl->m_bitmap;
}



void window_t::draw_pixels_bgr_front(const void* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	this->m_impl->draw_pixels(data, x, y, width, height, 24);
}
void window_t::draw_pixels_bgra_front(const void* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	this->m_impl->draw_pixels(data, x, y, width, height, 32);
}



void window_t::set_fullscreen_windowed() noexcept
{
	HMONITOR monitor = MonitorFromWindow(this->m_impl->m_window, MONITOR_DEFAULTTONEAREST);
	if (monitor == nullptr) return;

	auto window = this->m_impl->m_window;

	auto prev_size = this->get_client_size();
	auto prev_pos = this->get_client_position();

	MONITORINFO info;
	info.cbSize = sizeof(info);
	GetMonitorInfoA(monitor, &info);
	AdjustWindowRectEx(&info.rcMonitor, GetWindowLongA(window, GWL_STYLE), false, 0);
	SetWindowPos(window, HWND_TOP, info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right - info.rcMonitor.left, info.rcMonitor.bottom - info.rcMonitor.top, 0);

	auto new_size = this->get_client_size();
	auto new_pos = this->get_client_position();

	event_t ev;

	ev.type = event_type_t::resize;
	ev.window_resize_data.width_new = new_size.first;
	ev.window_resize_data.height_new = new_size.second;
	ev.window_resize_data.width_old = prev_size.first;
	ev.window_resize_data.height_old = prev_size.second;
	try
	{
		lock_event_queue(this->m_impl);
		this->m_impl->m_queue.push_back(ev);
	}
	catch (...) {}

	ev.type = event_type_t::move;
	ev.window_move_data.x_new = new_pos.first;
	ev.window_move_data.y_new = new_pos.second;
	ev.window_move_data.x_old = prev_pos.first;
	ev.window_move_data.y_old = prev_pos.second;
	try
	{
		lock_event_queue(this->m_impl);
		this->m_impl->m_queue.push_back(ev);
	}
	catch (...) {}
}

void window_t::set_cursor_capture(bool capture) noexcept
{
	this->m_impl->m_is_clipping = capture;

	if (!capture)
		return (void)ClipCursor(NULL);

	WINDOWINFO info;
	info.cbSize = sizeof(info);
	GetWindowInfo(this->m_impl->m_window, &info);
	ClipCursor(&info.rcClient);
}

bool window_t::set_title(const char* name) const noexcept
{
	return this->m_impl->set_title(name);
}
bool window_t::set_title(const wchar_t* name) const noexcept
{
	return this->m_impl->set_title(name);
}
bool window_t::set_title(const char8_t* name) const noexcept
{
	return this->m_impl->set_title(name);
}
bool window_t::set_title(const char16_t* name) const noexcept
{
	return this->m_impl->set_title(name);
}
bool window_t::set_title(const char32_t* name) const noexcept
{
	return this->m_impl->set_title(name);
}

bool window_t::has_focus() const noexcept
{
	return this->m_impl->m_window && GetActiveWindow() == this->m_impl->m_window;
}
void window_t::request_focus() const noexcept
{
	auto win = this->m_impl->m_window;
	if (!win) return;

	if (IsIconic(win))
		ShowWindow(win, SW_RESTORE);
	SetActiveWindow(win);
}

void window_t::set_cursor_visible(bool visible) const noexcept
{
	SetCursor(visible ? LoadCursorA(NULL, (LPCSTR)IDC_ARROW) : NULL);
}

void window_t::set_repeat_keyboard(bool enabled) noexcept
{
	this->m_impl->m_is_repetitive_keyboard_enabled = enabled;
}



bool window_t::set_size_constraint(uint16_t min_width, uint16_t min_height, uint16_t max_width, uint16_t max_height) noexcept
{
	return this->set_size_constraint({ min_width, min_height }, { max_width, max_height });
}
bool window_t::set_size_constraint(std::pair<uint16_t, uint16_t> min_size, std::pair<uint16_t, uint16_t> max_size) noexcept
{
	if (min_size.first > max_size.first || min_size.second > max_size.second)
		return false;

	int system_min_width = GetSystemMetricsClient(this->m_impl->m_window).width_min;
	if (min_size.first < system_min_width || max_size.first < system_min_width)
		return false;

	this->m_impl->m_size_min = min_size;
	this->m_impl->m_size_max = max_size;

	this->set_client_size(this->get_client_size());

	return true;
}
bool window_t::set_size_min_width(uint16_t x) noexcept
{
	if (x < GetSystemMetricsClient(this->m_impl->m_window).width_min)
		return false;

	this->m_impl->m_size_min.first = x;
	return true;
}
bool window_t::set_size_max_width(uint16_t x) noexcept
{
	if (x < GetSystemMetricsClient(this->m_impl->m_window).width_min)
		return false;
	if (x < this->m_impl->m_size_min.first)
		return false;

	this->m_impl->m_size_max.first = x;
	return true;
}
bool window_t::set_size_min_height(uint16_t x) noexcept
{
	this->m_impl->m_size_min.second = x;
	return true;
}
bool window_t::set_size_max_height(uint16_t y) noexcept
{
	if (y < this->m_impl->m_size_min.second)
		return false;

	this->m_impl->m_size_max.second = y;
	return true;
}
void window_t::set_special_keys_check_on_event(bool check_enabled) noexcept
{
	this->m_impl->m_check_special_keys_on_keyboard_event = check_enabled;
}

void window_t::set_resizemove_handle(window_resizemove_handle_t handle) noexcept
{
	this->m_impl->m_resizemove_handle = handle;
}
window_resizemove_handle_t window_t::get_resizemove_handle() const noexcept
{
	return this->m_impl->m_resizemove_handle;
}



void window_t::set_thread_safe_events(bool enabled) noexcept
{
	this->m_impl->m_is_thread_safe_events = enabled;
}

_KSN_END





//I don't remember why did i decide to not build that file and rather just include it here
//but it works and i'm a little bit scared to change anything
//I hope it will pay off when i will write implementations for other OS'es (if that day will ever come)
#include "libksn_window_impl_independend.cpp"
