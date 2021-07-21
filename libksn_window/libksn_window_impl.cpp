
#include <ksn/window.hpp>
#include <ksn/unicode.hpp>

#include <Windows.h>

#include <stdio.h>

#include <queue>
#include <codecvt>



_KSN_BEGIN



class window_t::_window_impl
{

	friend class window_t;



public:

	static LRESULT WINAPI __ksn_wnd_proc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
	{
		static constexpr bool is_wide = true;

		msg = LOWORD(msg);



		if (msg == WM_CREATE)
		{
			ksn::window_t::_window_impl& win_impl = *(window_t::_window_impl*)( ((CREATESTRUCTW*)(l))->lpCreateParams );
			std::deque<event_t>& q = win_impl.m_queue;

			SetWindowLongPtrW(wnd, GWLP_USERDATA, (LONG_PTR)&win_impl);

			event_t ev;
			ev.type = event_type_t::create;
			q.push_back(ev);

			return 0;
		}

		else if (msg == WM_GETMINMAXINFO)
		{
			MINMAXINFO* info = (MINMAXINFO*)l;
			info->ptMaxTrackSize.x = 65535;
			info->ptMaxTrackSize.y = 65535;
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
			q.push_back(ev); \
		}


		switch (msg)
		{
		case WM_CLOSE:
		{
			event_t ev;
			ev.type = event_type_t::close;

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
				q.push_back(ev);
			}
			else if (w == SIZE_MINIMIZED)
			{
				event_t ev;
				ev.type = event_type_t::minimized;
				q.push_back(ev);
			}
			else if (w == SIZE_RESTORED && win_impl.m_is_resizemove)
			{
				WINDOWINFO info;
				info.cbSize = sizeof(info);
				GetWindowInfo(win_impl.m_window, &info);
				ClipCursor(&info.rcClient);

				RECT client_area = info.rcClient;

				if (win_impl.m_is_clipping)
					ClipCursor(&client_area);

				if (win_impl.m_is_repetitive_resize_enabled)
				{
					uint16_t width = uint16_t(client_area.right - client_area.left);
					uint16_t height = uint16_t(client_area.bottom - client_area.top);

					auto new_size = std::pair<uint16_t, uint16_t>(width, height);

					if (new_size != win_impl.m_resizemove_last_size)
					{
						event_t ev;
						ev.type = event_type_t::resize;

						ev.window_resize_data.width_new = width;
						ev.window_resize_data.height_new = height;

						ev.window_resize_data.width_old = win_impl.m_resizemove_last_size.first;
						ev.window_resize_data.height_old = win_impl.m_resizemove_last_size.second;

						q.push_back(ev);

						win_impl.m_resizemove_last_size = new_size;
					}
				}
			}
		}
		break;

		case WM_MOVE:
		{
			if (win_impl.m_is_resizemove)
			{
				RECT client_area;
				GetClientRect(win_impl.m_window, &client_area);

				if (win_impl.m_is_repetitive_move_enabled)
				{
					auto new_pos = std::pair<int32_t, int32_t>((int32_t)client_area.left, (int32_t)client_area.top);

					if (new_pos != win_impl.m_resizemove_last_pos)
					{
						event_t ev;
						ev.type = event_type_t::move;

						ev.window_move_data.x_new = new_pos.first;
						ev.window_move_data.y_new = new_pos.second;
						ev.window_move_data.x_old = win_impl.m_resizemove_last_pos.first;
						ev.window_move_data.y_old = win_impl.m_resizemove_last_pos.second;

						q.push_back(ev);
					}
				}

			}
		}
		break;

		case WM_ENTERSIZEMOVE:
		{
			ClipCursor(NULL);

			RECT client_area;
			GetClientRect(win_impl.m_window, &client_area);

			win_impl.m_resizemove_last_pos.first = (int32_t)client_area.left;
			win_impl.m_resizemove_last_pos.second = (int32_t)client_area.bottom;

			win_impl.m_resizemove_last_size.first = (uint16_t)(client_area.right - client_area.left);
			win_impl.m_resizemove_last_size.second = (uint16_t)(client_area.bottom - client_area.top);

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

			if (new_pos != win_impl.m_resizemove_last_pos)
			{
				event_t ev;
				ev.type = event_type_t::move;

				ev.window_move_data.x_new = new_pos.first;
				ev.window_move_data.y_new = new_pos.second;

				ev.window_move_data.x_old = win_impl.m_resizemove_last_pos.first;
				ev.window_move_data.y_old = win_impl.m_resizemove_last_pos.second;

				q.push_back(ev);
			}
			if (new_size != win_impl.m_resizemove_last_size)
			{
				event_t ev;
				ev.type = event_type_t::resize;

				ev.window_resize_data.width_new = new_size.first;
				ev.window_resize_data.height_new = new_size.second;

				ev.window_resize_data.width_old = win_impl.m_resizemove_last_size.first;
				ev.window_resize_data.height_old = win_impl.m_resizemove_last_size.second;

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

					q.push_back(ev);

					win_impl.m_pending_wchar = 0;
				}
				else //Single UTF-16 unit
				{
					event_t ev;
					ev.type = event_type_t::text;
					ev.character = (uint32_t)w;

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
			bool repeated = HIWORD(l) & KF_REPEAT;
			if (win_impl.m_is_repetitive_keyboard_enabled || !repeated)
			{
				event_t ev;
				if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
				{
					if (repeated)
						ev.type = event_type_t::keyboard_hold;
					else
						ev.type = event_type_t::keyboard_press;
				}
				else ev.type = event_type_t::keyboard_release;

				ev.keyboard_button_data.button = my_t::_get_button(w, l);

				if (win_impl.m_check_special_keys_on_keyboard_event)
				{
					ev.keyboard_button_data.alt = HIWORD(GetKeyState(VK_MENU)) != 0;
					ev.keyboard_button_data.control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
					ev.keyboard_button_data.shift = HIWORD(GetKeyState(VK_SHIFT)) != 0;
					ev.keyboard_button_data.system = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
				}

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
				q.push_back(ev);
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

			q.push_back(ev);
		}
		break;


		default: break;
		};


		if ((msg == WM_SYSCOMMAND) && (w == SC_KEYMENU))
			return 0;


		auto* p_def_proc = is_wide ? DefWindowProcW : DefWindowProcA;
		return (*p_def_proc)(wnd, msg, w, l);

#undef push_mouse_event
#undef push_mouse_scroll_event
	}



	using my_t = window_t::_window_impl;


public:

	static HDC s_screen_hdc;


	std::deque<event_t> m_queue;
	HWND m_window;
	HDC m_hdc;
	HDC m_hmdc; //memory device context
	HBITMAP m_bitmap;
	std::pair<int32_t, int32_t> m_resizemove_last_pos;
	std::pair<uint16_t, uint16_t> m_resizemove_last_size;
	wchar_t m_pending_wchar;
	union
	{
		struct
		{
			bool m_is_repetitive_resize_enabled : 1; //event
			bool m_is_repetitive_move_enabled : 1; //event
			bool m_is_repetitive_keyboard_enabled : 1; //event
			bool m_is_resizemove : 1; //Whether WM_ENTERSIZEMOVE was received but WM_EXITSIZEMOVE still hasn't been
			bool m_check_special_keys_on_keyboard_event : 1;
			bool m_mouse_inside : 1;
			bool m_is_closing : 1;
			bool m_is_clipping : 1;
		};
		uint8_t m_bitfields;
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



public:

	_window_impl() noexcept
	{
		this->m_window = nullptr;
		this->m_hdc = nullptr;
		this->m_hmdc = nullptr;
		this->m_bitmap = nullptr;
		this->m_pending_wchar = 0;
		this->m_is_repetitive_move_enabled = false;
		this->m_is_repetitive_resize_enabled = false;
		this->m_is_repetitive_keyboard_enabled = true;
		this->m_is_resizemove = false;
		this->m_check_special_keys_on_keyboard_event = true;
		this->m_is_closing = false;
		this->m_is_clipping = false;
	}
	~_window_impl() noexcept
	{
		this->close();
	}


	void close() noexcept
	{
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
	window_open_result_t open(uint16_t width, uint16_t height, const char_t* window_name, window_style_t window_style) noexcept
	{
		//Just a wrapper function for "real" _Xopen

		this->close();
		window_open_result_t result = this->_Xopen(width, height, to_wchar(window_name).data(), window_style);

		if (result == window_open_result::ok || result == window_open_result::ok_but_direct_drawing_unsupported)
		{
			//Window is created hidden initially
			if (!(window_style & window_style::hidden)) ShowWindow(this->m_window, SW_SHOW);

			//Check whether mouse is initially inside the window
			{
				POINT cursor_pos;
				GetCursorPos(&cursor_pos);
				ScreenToClient(this->m_window, &cursor_pos);

				WINDOWINFO info;
				info.cbSize = sizeof(info);
				GetWindowInfo(this->m_window, &info); 
				
				this->m_mouse_inside =
					cursor_pos.x >= info.rcClient.left &&
					cursor_pos.x < info.rcClient.right&&
					cursor_pos.y >= info.rcClient.top &&
					cursor_pos.y < info.rcClient.bottom;

				this->m_resizemove_last_size.first = uint16_t(info.rcClient.right - info.rcClient.left);
				this->m_resizemove_last_size.second = uint16_t(info.rcClient.bottom - info.rcClient.top);
				
				this->m_resizemove_last_pos.first = info.rcClient.left;
				this->m_resizemove_last_pos.second = info.rcClient.top;
			}

			//Set mouse tracking for WM_MOUSELEAVE
			{
				TRACKMOUSEEVENT track_info;
				track_info.cbSize = sizeof(track_info);
				track_info.dwFlags = TME_LEAVE;
				track_info.hwndTrack = this->m_window;
				TrackMouseEvent(&track_info);
			}

			{
				//Get rid of all "default" messages
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
				_KSN_DEBUG_EXPR(if (threshold < 0) printf("THRESHOLD REACHED\n\a"));
			}
		}
		else
		{
			//If something has broken, undid everything that hasn't but preserve the winapi error
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
		
		{
			//SetWindowPos(this->m_window, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);

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
		if (width == uint16_t(-1)) width = this->m_resizemove_last_size.first;
		if (height == uint16_t(-1)) height = this->m_resizemove_last_size.second;

		BITMAPINFO bitmapinfo{};
		bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapinfo.bmiHeader.biWidth = (int)width;
		bitmapinfo.bmiHeader.biHeight = -(int)height;
		bitmapinfo.bmiHeader.biPlanes = 1;
		bitmapinfo.bmiHeader.biBitCount = bits;
		bitmapinfo.bmiHeader.biCompression = BI_RGB;

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
};



HDC window_t::_window_impl::s_screen_hdc = GetDC(nullptr);




namespace
{
	struct __library_constructor_t
	{
		__library_constructor_t() noexcept
		{
			WNDCLASSW wcW{};
			wcW.lpszClassName = L"_LIBKSN_window_";
			wcW.lpfnWndProc = window_t::_window_impl::__ksn_wnd_proc;
			RegisterClassW(&wcW);
		}

		~__library_constructor_t() noexcept
		{
			ReleaseDC(nullptr, window_t::_window_impl::s_screen_hdc);
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



window_open_result_t window_t::open(uint16_t width, uint16_t height, const char* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const wchar_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const char8_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const char16_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style);
}
window_open_result_t window_t::open(uint16_t width, uint16_t height, const char32_t* title, window_style_t window_style) noexcept
{
	return this->m_impl->open(width, height, title, window_style);
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

void window_t::discard_all_events() noexcept
{
	this->discard_stored_events();
	this->m_impl->m_queue.clear();
}
void window_t::discard_stored_events() noexcept
{
	this->m_impl->m_queue.clear();
	MSG native;
	while (PeekMessageW(&native, this->m_impl->m_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&native);
		DispatchMessageW(&native);
	}
}

bool window_t::is_open() const noexcept
{
	return this->m_impl->is_open();
}

std::pair<uint16_t, uint16_t> window_t::get_client_size() const noexcept
{
	WINDOWINFO wi;
	wi.cbSize = sizeof(wi);
	if (!GetWindowInfo(this->m_impl->m_window, &wi)) return { 0, 0 };
	return { uint16_t(wi.rcClient.right - wi.rcClient.left), uint16_t(wi.rcClient.bottom - wi.rcClient.top) };
}

void window_t::set_client_size(uint16_t width, uint16_t height) const noexcept
{
	RECT size = { 0, 0, (long)width, (long)height };
	auto window = this->m_impl->m_window;
	AdjustWindowRectEx(&size, GetWindowLongA(window, GWL_STYLE), false, 0);
	SetWindowPos(window, NULL, 0, 0, (uint16_t)(size.right - size.left), (uint16_t)(size.bottom - size.top), SWP_NOMOVE | SWP_NOZORDER);
}

std::pair<int16_t, int16_t> window_t::get_client_position() const noexcept
{
	WINDOWINFO wi;
	wi.cbSize = sizeof(wi);
	if (!GetWindowInfo(this->m_impl->m_window, &wi)) return { 0, 0 };
	return { int16_t(wi.rcClient.left), int16_t(wi.rcClient.top) };
}

void window_t::set_client_position(int16_t x, int16_t y) const noexcept
{
	RECT pos;
	auto window = this->m_impl->m_window;
	GetClientRect(window, &pos);

	pos.right += x - pos.left;
	pos.bottom += y - pos.top;
	pos.left = x;
	pos.top = y;

	AdjustWindowRectEx(&pos, GetWindowLongA(window, GWL_STYLE), false, 0);
	SetWindowPos(window, NULL, pos.left, pos.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
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
}


HDC window_t::winapi_get_hdc() const noexcept
{
	return this->m_impl->m_hdc;
}



void window_t::draw_pixels_bgr_front(const void* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	this->m_impl->draw_pixels(data, x, y, width, height, 24);
}
void window_t::draw_pixels_bgra_front(const void* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	this->m_impl->draw_pixels(data, x, y, width, height, 32);
}



void window_t::set_fullscreen_windowed() const noexcept
{
	HMONITOR monitor = MonitorFromWindow(this->m_impl->m_window, MONITOR_DEFAULTTONEAREST);
	if (monitor == nullptr) return;

	auto window = this->m_impl->m_window;

	MONITORINFO info;
	info.cbSize = sizeof(info);
	GetMonitorInfoA(monitor, &info);
	AdjustWindowRectEx(&info.rcMonitor, GetWindowLongA(window, GWL_STYLE), false, 0);
	SetWindowPos(window, HWND_TOP, info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right - info.rcMonitor.left, info.rcMonitor.bottom - info.rcMonitor.top, 0);
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


_KSN_END

#include "libksn_window_impl_independend.cpp"
