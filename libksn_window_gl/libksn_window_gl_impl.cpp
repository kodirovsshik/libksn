
#include <ksn/window_gl.hpp>

#include <Windows.h>

#include <GL/glew.h>
#include <GL/wglew.h>

#include <utility>



_KSN_BEGIN



class window_gl_t::_window_gl_impl
{

	friend class window_gl_t;


public:

	HGLRC m_context;



	_window_gl_impl() noexcept
	{
		this->m_context = nullptr;
	}
	~_window_gl_impl() noexcept
	{
		this->context_delete();
	}

	_window_gl_impl(const _window_gl_impl&) = delete;
	_window_gl_impl(_window_gl_impl&& other) noexcept
	{
		std::swap(this->m_context, other.m_context);
	}

	_window_gl_impl& operator=(const _window_gl_impl&) = delete;
	_window_gl_impl& operator=(_window_gl_impl&& other) noexcept
	{
		std::swap(this->m_context, other.m_context);
		return *this;
	}



	void context_delete() noexcept
	{
		if (this->m_context) wglDeleteContext(this->m_context);
		this->m_context = nullptr;
	}

	window_open_result_t context_create(context_settings settings, window_gl_t* win)
	{
		if (settings.ogl_version_major == 0) return window_open_result::ok;

		HDC hdc = win->winapi_get_device_context();
		if (!_process_pfd(hdc, settings.bits_per_color)) return window_open_result::system_error;

		//For OpenGL version > 1.1 use ARB extension to create a context
		if (settings.ogl_version_major > 1 || (settings.ogl_version_major == 1 && settings.ogl_version_minor > 1))
		{
			//Create temporary context and make it current so glew can initialize
			HGLRC temp_context = wglCreateContext(hdc);
			if (temp_context == nullptr) return window_open_result::opengl_error;

			HGLRC previous_context = wglGetCurrentContext();
			HDC previous_hdc = wglGetCurrentDC();

			wglMakeCurrent(hdc, temp_context);

			glewExperimental = true; //Make GLEW do it's best
			auto glew_init_result = glewInit();

			wglMakeCurrent(previous_hdc, previous_context);
			wglDeleteContext(temp_context);

			if (glew_init_result != GLEW_OK) return window_open_result::glew_error;

			const auto& [v_maj, v_min, bpp, compatibility, debug] = settings;
			//If extension is available
			if (wglCreateContextAttribsARB)
			{
				int attributes[] =
				{
					WGL_CONTEXT_MAJOR_VERSION_ARB, v_maj,
					WGL_CONTEXT_MINOR_VERSION_ARB, v_min,
					WGL_CONTEXT_PROFILE_MASK_ARB, compatibility
						? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
						: WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
					WGL_CONTEXT_FLAGS_ARB, debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
					0
				};

				this->m_context = wglCreateContextAttribsARB(hdc, nullptr, attributes);
				return this->m_context ? window_open_result::ok : window_open_result::opengl_error;
			}
			else if (compatibility || v_maj < 3 || (v_maj == 3 && v_min < 2))
			{
				//Drivers may be nice to us and create a modern compatibitity context for us by default
				this->m_context = wglCreateContext(hdc);

				if (this->m_context == nullptr) return window_open_result::opengl_error;

				HDC last_dc = wglGetCurrentDC();
				HGLRC last_context = wglGetCurrentContext();
				wglMakeCurrent(hdc, this->m_context);


				int ogl_major = -1, ogl_minor = -1;

				do
				{
					glGetIntegerv(GL_MAJOR_VERSION, &ogl_major);
					glGetIntegerv(GL_MINOR_VERSION, &ogl_minor);

					if ((ogl_major | ogl_minor) != -1) break;


					if (glGetInteger64v)
					{
						GLint64 ogl_major64 = -1, ogl_minor64 = -1;
						glGetInteger64v(GL_MAJOR_VERSION, &ogl_major64);
						glGetInteger64v(GL_MINOR_VERSION, &ogl_minor64);
						ogl_major = (int)ogl_major64;
						ogl_minor = (int)ogl_minor64;
					}

					if ((ogl_major | ogl_minor) != -1) break;

					const char* version = (const char*)glGetString(GL_VERSION);
					if (version)
					{
						int result = sscanf_s(version, "%i.%i", &ogl_major, &ogl_minor);
						if (result == 2 && (ogl_major | ogl_minor) != -1) break;
					}

				} while (false);

				wglMakeCurrent(last_dc, last_context);

				if ((ogl_major | ogl_minor) == -1) return window_open_result::opengl_unsupported_function;

				if (ogl_major < v_maj || ogl_major == v_maj && ogl_minor < v_min) return window_open_result::opengl_error;
				return window_open_result::ok;
			}
			else
			{
				//wglCreateContextAttribsARB is unavailable, no way to create modern core context
				return window_open_result::opengl_unsupported_function;
			}
		}
		else if (settings.ogl_version_major > 0) //otherwise (if needed OpenGL 1.1), use native WGL
		{
			this->m_context = wglCreateContext(hdc);
			if (this->m_context == nullptr) return window_open_result::opengl_error;

			HGLRC previous_context = wglGetCurrentContext();
			HDC previous_hdc = wglGetCurrentDC();

			wglMakeCurrent(hdc, this->m_context);

			glewExperimental = true;
			glewInit();

			wglMakeCurrent(previous_hdc, previous_context);

			return window_open_result::ok;
		}
		else //otherwise we don't need a context at all
			return window_open_result::ok;
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
	window_open_result_t open(uint16_t width, uint16_t height, const char_t* window_name, window_gl_t::context_settings settings, window_style_t window_style, window_gl_t* win) noexcept
	{
		window_open_result_t result = this->_Xopen(width, height, window_name, settings, window_style, win);

		if (result != window_open_result::ok && result != window_open_result::ok_but_direct_drawing_unsupported)
		{
			this->context_delete();
		}

		return result;
	}

	template<typename char_t>
	window_open_result_t _Xopen(uint16_t width, uint16_t height, const char_t* window_name, window_gl_t::context_settings settings, window_style_t window_style, window_gl_t* win) noexcept
	{
		if (memcmp(&settings, &window_gl_t::opengl_no_context, sizeof(window_gl_t::context_settings)) != 0) //If we need a context
			window_style |= (1 << (sizeof(window_style_t) * CHAR_BIT - 1)); //Ask the base class not to set the pixel format

		window_open_result_t result = win->window_t::open(width, height, window_name, window_style);
		if (result == window_open_result::ok || result == window_open_result::ok_but_direct_drawing_unsupported)
		{
			window_open_result_t context_create_result = this->context_create(settings, win);
			if (context_create_result != window_open_result::ok) return context_create_result;
		}
		return result;
	}
};

void window_gl_t::close() noexcept
{
	this->m_gl_impl->context_delete();
	window_t::close();
}

window_gl_t::window_gl_t() noexcept = default;
window_gl_t::window_gl_t(window_gl_t&& other) noexcept
{
	this->m_gl_impl = std::move(other.m_gl_impl);
}
window_gl_t::window_gl_t(uint16_t width, uint16_t height, const char* title, context_settings settings, window_style_t style) noexcept
{
	this->m_gl_impl->open(width, height, title, settings, style, this);
}
window_gl_t::window_gl_t(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings, window_style_t style) noexcept
{
	this->m_gl_impl->open(width, height, title, settings, style, this);
}
window_gl_t::~window_gl_t() noexcept = default;

window_open_result_t window_gl_t::open(uint16_t width, uint16_t height, const char* title, context_settings settings, window_style_t style) noexcept
{
	return this->m_gl_impl->open(width, height, title, settings, style, this);
}
window_open_result_t window_gl_t::open(uint16_t width, uint16_t height, const wchar_t* title, context_settings settings, window_style_t style) noexcept
{
	return this->m_gl_impl->open(width, height, title, settings, style, this);
}



void window_gl_t::swap_buffers() const noexcept
{
	//SwapBuffers(this->m_impl->m_hdc);
	wglSwapLayerBuffers(this->winapi_get_device_context(), WGL_SWAP_MAIN_PLANE);
}



HGLRC window_gl_t::context_native_handle() const noexcept
{
	return this->m_gl_impl->m_context;
}
void window_gl_t::context_make_current() const noexcept
{
	if (this->m_gl_impl->m_context) wglMakeCurrent(this->winapi_get_device_context(), this->m_gl_impl->m_context);
}
bool window_gl_t::context_is_current() const noexcept
{
	auto my_context = this->m_gl_impl->m_context;
	return my_context && wglGetCurrentContext() == my_context;
}


bool window_gl_t::context_present() const noexcept
{
	return this->m_gl_impl->m_context != nullptr;
}

_KSN_END
