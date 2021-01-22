
#include <Windows.h>

#include <ksn/graphics_engine_window.hpp>

#include <deque>
#include <thread>
#include <semaphore>

#include <GL/glew.h>
#include <GL/wglew.h>



#pragma warning(disable : 26812)



_KSN_BEGIN


struct window_t::_window_impl
{
	using my_t = window_t::_window_impl;



	static HDC s_screen_hdc;
	static bool glew_initialized;

	std::deque<MSG> m_msgs_q;
	std::jthread m_msgs_thread;
	std::binary_semaphore* m_msgs_p_lock;
	HWND m_window;
	HGLRC m_context;
	HDC m_hdc;



	_window_impl() noexcept
	{
		this->m_window = nullptr;
		this->m_hdc = nullptr;
		this->m_context = nullptr;
		this->m_msgs_p_lock = nullptr;
	}

	~_window_impl()
	{
		this->close();

		delete this->m_msgs_p_lock;
	}

	static void msg_processor(std::stop_token st, my_t* window)
	{
		MSG msg;
		while (!st.stop_requested() && GetMessageW(&msg, window->m_window, 0, 0) == 1)
		{
			window->m_msgs_p_lock->acquire();
			window->m_msgs_q.push_back(std::move(msg));
			window->m_msgs_p_lock->release();
		}
		int a = 0;
		a = a;
	}

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



	template<typename char_t>
	bool open(size_t width, size_t height, const char_t* window_name, window_t::context_settings settings, window_t::style_t style)
	{
		bool ok = this->_Xopen(width, height, window_name, settings, style);
		if (ok)
		{
			if (this->m_msgs_p_lock == nullptr) this->m_msgs_p_lock = new std::binary_semaphore(1);
			this->m_msgs_thread = std::jthread(msg_processor, this);
			if (wglGetCurrentContext() == nullptr) wglMakeCurrent(this->m_hdc, this->m_context);
		}
		else
		{
#if _DEBUG
			printf("Err %i %i\n", GetLastError(), glGetError());
#endif
			this->close();
		}
		return ok;
	}
	template<typename char_t>
	bool _Xopen(size_t width, size_t height, const char_t* window_name, window_t::context_settings settings, window_t::style_t style)
	{
		constexpr static bool is_wide = std::is_same_v<wchar_t, char_t>;
		constexpr static const char_t* const class_name = !is_wide 
#pragma warning(push)
#pragma warning(disable : 6276)
			? (const char_t*)("_KSN_")
			: (const char_t*)(L"_KSN_");
#pragma warning(pop)

		static_assert(std::is_same_v<char_t, char> || std::is_same_v<char_t, wchar_t>);



		if (style & window_t::style_t::fullscreen) return false; //TODO

		std::conditional_t<!is_wide, WNDCLASSA, WNDCLASSW> wc{};
		wc.lpfnWndProc = &DefWindowProcA;
		wc.lpszClassName = class_name;

		ATOM register_result;
		if constexpr (!is_wide)
		{
			register_result = RegisterClassA(&wc);
		}
		else
		{
			register_result = RegisterClassW(&wc);
		}
		if (register_result == 0) return false;
		
		/*
		Border = 1 //WS_BORDER
		Close button = 2 //WS_SYSMENU
		Minmax buttons = 4 //WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU
		Resize = 8 //WS_THICKFRAM
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
		
		DWORD winapi_style = WS_VISIBLE;
		for (int i = 0; i < 8; ++i) if (style & (1 << i)) winapi_style |= winapi_flags[i];

		if constexpr (!is_wide)
		{
			this->m_window = CreateWindowA(class_name, window_name, winapi_style, CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height, nullptr, nullptr, nullptr, this);
		}
		else
		{
			this->m_window = CreateWindowW(class_name, window_name, winapi_style, CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height, nullptr, nullptr, nullptr, this);
		}

		if (this->m_window == nullptr) return false;

		this->m_hdc = GetDC(this->m_window);
		if (this->m_hdc == nullptr) return false;

		if (settings.ogl_version_major > 1 || (settings.ogl_version_major == 1 && settings.ogl_version_minor > 1))
		{
			//Need some extentions
			if (my_t::glew_initialized == false)
			{
				//Create temporary context and make it current so glew can load without bugs
				
				if (!my_t::_process_pfd(this->m_hdc, settings.bits_per_color)) return false;

				HGLRC temp = wglCreateContext(this->m_hdc);
				if (temp == nullptr) return false;

				HGLRC previous_context = wglGetCurrentContext();
				HDC previous_hdc = wglGetCurrentDC();

				wglMakeCurrent(this->m_hdc, temp);

				glewExperimental = true;
				auto glew_init_result = glewInit();

				wglMakeCurrent(previous_hdc, previous_context);
				wglDeleteContext(temp);

				if (glew_init_result != GLEW_OK) return false;

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
				return this->m_context != nullptr;
			}

			return false;
		}
		else
		{
			if (!my_t::_process_pfd(this->m_hdc, settings.bits_per_color)) return false;

			this->m_context = wglCreateContext(this->m_hdc);
			return this->m_context != nullptr;
		}
	}

	void close()
	{
		//this->m_msgs_thread.request_stop();
		
		//CloseWindow(this->m_window);
		wglDeleteContext(this->m_context);
		ReleaseDC(this->m_window, this->m_hdc);
		DestroyWindow(this->m_window);

		//if (this->m_msgs_thread.joinable())
		{
			//this->m_msgs_thread.join();
			//Sleep(100);
			//this->m_msgs_thread.join();
		}
	}
};


HDC window_t::_window_impl::s_screen_hdc = GetDC(nullptr);
bool window_t::_window_impl::glew_initialized = false;



window_t::~window_t()
{
	this->m_impl.ptr()->~_window_impl();
}
window_t::window_t() noexcept
{
	new (&this->m_impl) decltype(this->m_impl);
}
window_t::window_t(size_t width, size_t height, const char* title, context_settings settings, style_t style) noexcept
{
	new (&this->m_impl) decltype(this->m_impl);
	this->m_impl->open(width, height, title, settings, style);
}
window_t::window_t(size_t width, size_t height, const wchar_t* title, context_settings settings, style_t style) noexcept
{
	new (&this->m_impl) decltype(this->m_impl);
	this->m_impl->open(width, height, title, settings, style);
}



_KSN_END
