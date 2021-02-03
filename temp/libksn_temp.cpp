
#include <ksn/stuff.hpp>
#include <ksn/math_constants.hpp>

#include <ksn/window.hpp>
#include <ksn/graphics_engine.hpp>

#include <Windows.h>
#include <GL/GL.h>

#include <exception>

#include <stdio.h>


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "opencl.lib")
#pragma comment(lib, "glew32s.lib")

//OpenCL dependencies
#pragma comment(lib, "RuntimeObject.lib")
#pragma comment(lib, "Cfgmgr32.lib")

#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_graphics_engine.lib")
//#pragma comment(lib, "libksn_stuff.lib")
//#pragma comment(lib, "libksn_x86_instruction_set.lib")



_KSN_BEGIN

[[noreturn]] void __cdecl long_jump(const void*);

_KSN_END


#pragma warning(disable : 4996)

float sine_rough(float x)
{
	x = fmod(x, KSN_PIf * 2);
	bool neg;
	if (x > KSN_PIf)
	{
		neg = true;
		x -= KSN_PIf;
	}
	else
	{
		neg = false;
	}

	float result;
	if (x < 0.54f) result = 0.96f * x;
	else if (x < 2.65f)
	{
		float temp = x - KSN_PIf / 2;
		result = -0.46f * temp * temp + 1;
	}
	else result = 0.95f * (KSN_PIf - x);

	if (neg) return -result;
	return result;
}



int main()
{

	int x[10];
	auto p = std::launder(&x[0]);
	ksn::graphics_engine_t ge;

	return 0;
}

//GraphicsFW
int main1()
{
	ksn::window_t win;
	ksn::window_t::context_settings settings{ 4, 6 };
	
	auto* pname = "libKSN window system";

	if (win.open(400, 300, pname, settings) != ksn::window_t::error::ok)
	{
		char buffer[256];
		snprintf(buffer, 256, "Failed to open a window\n\n\
You may have an old video card that does not support OpenGL versions > 3.1\n\
If you sure your videocard does, update your video card drivers\n\n\
WinAPI error status: 0x%08X\nOpenGL error status: 0x%08X\n", (int)GetLastError(), glGetError());
		MessageBoxA(0, buffer, "Error", MB_ICONERROR);

		return 1;
	}
	else
	{
		win.make_current();
		printf("Running OpenGL %s on %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));
	}

	

	ksn::event_t ev;
	while (win.is_open())
	{


		//Main loop



		Sleep(50);
		while (win.poll_event(ev))
		{
			if (ev.type != ksn::event_type_t::mouse_move) ev.dump();

			//printf("m_mouse_inside = %i\n", (int32_t)bool(((uint8_t*)&win.m_impl)[78] & 32));

			if (ev.type == ksn::event_type_t::close)
			{
				win.close();
			}
			else if (ev.type == ksn::event_type_t::keyboard_press)
			{
				if (ev.keyboard_button_data.button == ksn::event_t::keyboard_button_t::esc)
				{
					win.close();
				}
			}
		}
	}

	return 0;

}
