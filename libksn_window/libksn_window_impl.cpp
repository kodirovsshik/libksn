
#include <ksn/window.hpp>

#include <Windows.h>

#include <GL/glew.h>
#include <GL/wglew.h>

#include <stdio.h>

#include <atomic>



#pragma warning(disable : 26812 4996 26451 6001)



_KSN_BEGIN


class window_t::_window_impl
{
public:
	static LRESULT WINAPI __ksn_wnd_procA(HWND wnd, UINT msg, WPARAM w, LPARAM l)
	{
		printf("ProcA handled %04X: %016zX %016zX\n", LOWORD(msg), (size_t)w, (size_t)l);
		return DefWindowProcA(wnd, msg, w, l);
	}
	static LRESULT WINAPI __ksn_wnd_procW(HWND wnd, UINT msg, WPARAM w, LPARAM l)
	{
		printf("ProcW handled %04X: %016zX %016zX\n", LOWORD(msg), (size_t)w, (size_t)l);
		return DefWindowProcW(wnd, msg, w, l);
	}

	using my_t = window_t::_window_impl;
	using error_t = window_t::error_t;



	static HDC s_screen_hdc;
	static bool glew_initialized;
	static std::atomic_size_t window_counter;

	HWND m_window;
	HGLRC m_context;
	HDC m_hdc;
	size_t m_number;



//private:

	static bool _process_pfd(HDC hdc, int bpp)
	{
		PIXELFORMATDESCRIPTOR pfd{};
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = bpp;
		pfd.iLayerType = PFD_MAIN_PLANE;

		return SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
	}

	static void _process_msg(MSG& msg)
	{
		if (LOWORD(msg.message) != WM_DESTROY)
		{
			//printf("a");
			TranslateMessage(&msg);
			//printf("b");
			DispatchMessageW(&msg);
			//printf("c");
		}
	}



public:

	_window_impl() noexcept
	{
		this->m_window = nullptr;
		this->m_hdc = nullptr;
		this->m_context = nullptr;
		this->m_number = 0;
	}
	~_window_impl() noexcept
	{
		this->close();
	}


	void close() noexcept
	{
		if (this->m_context)
		{
			wglDeleteContext(this->m_context);
			this->m_context = nullptr;
		}
		if (this->m_hdc)
		{
			ReleaseDC(this->m_window, this->m_hdc);
			this->m_hdc = nullptr;
		}
		if (this->m_window)
		{
			DestroyWindow(this->m_window);
			this->m_window = nullptr;
		}
		if (this->m_number)
		{
			char buffer[32];
			snprintf(buffer, 32, "__KSNWINDOW_%016zX", this->m_number);
			UnregisterClassA(buffer, GetModuleHandleA(nullptr));
			this->m_number = 0;
		}
	}


	template<typename char_t>
	error_t open(uint16_t width, uint16_t height, const char_t* window_name, window_t::context_settings settings, window_t::style_t style) noexcept
	{
		this->close();
		error_t result = this->_Xopen(width, height, window_name, settings, style);

		if (result == error_t::ok)
		{
			if (wglGetCurrentContext() == nullptr) wglMakeCurrent(this->m_hdc, this->m_context);
			ShowWindow(this->m_window, SW_SHOW);
			
			//Get rid of all "default" messages
			MSG msg;
			int threshold = 32; //But don't keep going for too long
			while (threshold --> 0)
			{
				if (PeekMessageW(&msg, this->m_window, 0, 0, PM_REMOVE) <= 0)
				{
					break;
				}
				_KSN_DEBUG_EXPR(printf("0x%08X\n", msg.message));
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			_KSN_DEBUG_EXPR(if (threshold < 0) printf("THRESHOLD REACHED\n\a"));
			
		}
		else
		{
			int __error = GetLastError();
			this->close();
			SetLastError(__error);
		}
		return result;
	}

	template<typename char_t>
	error_t _Xopen(uint16_t width, uint16_t height, const char_t* window_name, window_t::context_settings settings, window_t::style_t style) noexcept
	{
		constexpr static bool is_wide = std::is_same_v<wchar_t, char_t>;
		static_assert(std::is_same_v<char_t, char> || std::is_same_v<char_t, wchar_t>);

		char_t class_name[32];


		if (style & window_t::style_t::fullscreen) return error_t::unimplemented; //TODO

		std::conditional_t<!is_wide, WNDCLASSA, WNDCLASSW> wc{};
		wc.lpszClassName = class_name;
		wc.hCursor = LoadCursorA(nullptr, (LPCSTR)IDC_ARROW);
		if constexpr (!is_wide)
		{
			wc.lpfnWndProc = &__ksn_wnd_procA;
		}
		else
		{
			wc.lpfnWndProc = &__ksn_wnd_procW;
		}
		//wc.lpfnWndProc = &noop_wnd_proc;

		this->m_number = ++my_t::window_counter;
		if constexpr (!is_wide)
		{
			sprintf_s(class_name, 32, "__KSNWINDOW_%016zX", this->m_number);
		}
		else
		{
			swprintf_s(class_name, 32, L"__KSNWINDOW_%016zX", this->m_number);
		}

		ATOM register_result;
		if constexpr (!is_wide)
		{
			register_result = RegisterClassA(&wc);
		}
		else
		{
			register_result = RegisterClassW(&wc);
		}
		if (register_result == 0) return error_t::system_error;

		/*
		Border = 1 //WS_BORDER
		Close button = 2 //WS_SYSMENU
		Minmax buttons = 4 //WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU
		Resize = 8 //WS_THICKFRAME
		Caption = 16 //WS_CAPTION
		Fullscreen = 32
		*/
		constexpr static UINT winapi_flags[] =
		{
			WS_BORDER,
			WS_SYSMENU | WS_CAPTION,
			WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU,
			WS_THICKFRAME,
			WS_CAPTION,
			0,
			0,
			0
		};
		
		DWORD winapi_style = 0;
		for (int i = 0; i < 8; ++i) if (style & (1 << i)) winapi_style |= winapi_flags[i];

		RECT rect{ 0, 0, (LONG)width, (LONG)height };
		if (style & ~style_t::fullscreen) //if not fullscreen
		{
			AdjustWindowRectEx(&rect, winapi_style, FALSE, 0);
			width = uint16_t(rect.right - rect.left);
			height = uint16_t(rect.bottom - rect.top);
		}
		
		
		if constexpr (!is_wide)
		{
			this->m_window = CreateWindowA(class_name, window_name, winapi_style, CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height, nullptr, nullptr, nullptr, this);
		}
		else
		{
			this->m_window = CreateWindowW(class_name, window_name, winapi_style, CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height, nullptr, nullptr, nullptr, this);
		}

		if (this->m_window == nullptr) return error_t::window_creation_error;

		{
			RECT actual_rect;
			GetWindowRect(this->m_window, &actual_rect);
			if (width != (actual_rect.right - actual_rect.left) || height != (actual_rect.bottom - actual_rect.top)) return error_t::window_size_error;
		}


		this->m_hdc = GetDC(this->m_window);
		if (this->m_hdc == nullptr) return error_t::system_error;

		if (settings.ogl_version_major > 1 || (settings.ogl_version_major == 1 && settings.ogl_version_minor > 1))
		{
			if (my_t::glew_initialized == false)
			{
				//Create temporary context and make it current so glew can load without bugs

				if (!my_t::_process_pfd(this->m_hdc, settings.bits_per_color)) return  error_t::system_error;

				HGLRC temp = wglCreateContext(this->m_hdc);
				if (temp == nullptr) return error_t::opengl_error;

				HGLRC previous_context = wglGetCurrentContext();
				HDC previous_hdc = wglGetCurrentDC();

				wglMakeCurrent(this->m_hdc, temp);

				glewExperimental = true;
				auto glew_init_result = glewInit();

				wglMakeCurrent(previous_hdc, previous_context);
				wglDeleteContext(temp);

				if (glew_init_result != GLEW_OK) return error_t::glew_error;

				my_t::glew_initialized = true;
			}

			if (wglCreateContextAttribsARB)
			{
				int attributes[] =
				{
					WGL_CONTEXT_MAJOR_VERSION_ARB, settings.ogl_version_major,
					WGL_CONTEXT_MINOR_VERSION_ARB, settings.ogl_version_minor,
					WGL_CONTEXT_PROFILE_MASK_ARB, settings.ogl_compatibility_profile
						? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
						: WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
					WGL_CONTEXT_FLAGS_ARB, settings.ogl_debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
					0
				};

				this->m_context = wglCreateContextAttribsARB(this->m_hdc, nullptr, attributes);
				return (this->m_context) ? error_t::ok : error_t::opengl_error;
			}

			return opengl_unsupported_function;
		}
		else
		{
			if (!my_t::_process_pfd(this->m_hdc, settings.bits_per_color)) return error_t::system_error;

			this->m_context = wglCreateContext(this->m_hdc);
			return (this->m_context) ? error_t::ok : error_t::opengl_error;
		}
	}

	bool is_open() const noexcept
	{
		WINDOWINFO _;
		return GetWindowInfo(this->m_window, &_) == TRUE;
	}
};



HDC window_t::_window_impl::s_screen_hdc = GetDC(nullptr);
bool window_t::_window_impl::glew_initialized = false;
std::atomic_size_t window_t::_window_impl::window_counter = 0;





window_t::native_window_t window_t::window_native_handle() const noexcept
{
	return this->m_impl->m_window;
}
window_t::native_context_t window_t::context_native_handle() const noexcept
{
	return this->m_impl->m_context;
}


window_t::window_t(uint16_t width, uint16_t height, const char* title, context_settings settings, style_t style) noexcept
{
	this->m_impl->open(width, height, title, settings, style);
}
window_t::window_t(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings, style_t style) noexcept
{
	this->m_impl->open(width, height, title, settings, style);
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


window_t::error_t window_t::open(uint16_t width, uint16_t height, const char* title, context_settings settings, style_t style) noexcept
{
	return this->m_impl->open(width, height, title, settings, style);
}
window_t::error_t window_t::open(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings, style_t style) noexcept
{
	return this->m_impl->open(width, height, title, settings, style);
}


void window_t::close() noexcept
{
	this->m_impl->close();
}

bool window_t::is_open() const noexcept
{
	return this->m_impl->is_open();
}



FILE* f = stdout;
//FILE* f = fopen("log.txt", "w");
bool window_t::poll_native_event(MSG& msg) const noexcept
{
	bool got_message = PeekMessageA(&msg, this->m_impl->m_window, 0, 0, PM_REMOVE) > 0;
	if (!got_message) return false;
	//fprintf(f, "0x%08X\n", (int)msg.message);
	this->m_impl->_process_msg(msg);
	return true;
	
}
bool window_t::wait_native_event(MSG& msg) const noexcept
{
	bool got_message = GetMessageW(&msg, this->m_impl->m_window, 0, 0) >= 0;
	if (!got_message) return false;
	//fprintf(f, "0x%08X\n", (int)msg.message);
	this->m_impl->_process_msg(msg);
	return true;
}


_KSN_END