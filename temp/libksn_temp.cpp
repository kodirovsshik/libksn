
//#include <ksn/stuff.hpp>
#include <ksn/math_constants.hpp>
//#include <ksn/debug_utils.hpp>

#include <ksn/window.hpp>
#include <ksn/graphics.hpp>

#include <Windows.h>
#include <GL/GL.h>

#include <exception>
#include <vector>

#include <stdio.h>


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "opencl.lib")
#pragma comment(lib, "glew32s.lib")

//OpenCL dependencies
#pragma comment(lib, "RuntimeObject.lib")
#pragma comment(lib, "Cfgmgr32.lib")

#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_graphics_engine.lib")
#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")


#pragma warning(disable : 26451 4530)



_KSN_BEGIN

[[noreturn]] void __cdecl long_jump(const void*);

_KSN_END


#pragma warning(disable : 4996)



//niffleheim
/*
https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapvirtualkeya
*/
int main()
{

	ksn::graphics::shape_buffer_t sb;
	ksn::graphics::vertex3_t vertexes[3] =
	{
		{ -0.5f, -0.5f, 0},
		{ 0.5f, -0.5f, 0},
		{ 0, 0.5f, 0},
	};
	uint32_t off = sb.registrate(vertexes, 3);
	ksn::graphics::surface_indexed_t triangle_surface = { 0, 1, 2 };
	uint32_t triangle_handle = sb.registrate(&triangle_surface, 1, off);



	ksn::graphics::color_t obama_data[6];

	ksn::graphics::texture_t obama;
	obama.data = obama_data;
	obama.w = 3;
	obama.h = 2;
	uint32_t obama_handle = sb.registrate(&obama, 1);
	//temp = sb.registrate(&obama, 0);

	sb.flush();

//	return main1();
//}
//
//
//int main1()
//{


	//GD bonfire

	ksn::window_t win;
	ksn::window_t::context_settings settings{ 3, 1, false };
	ksn::window_t::context_settings settings11;
	
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



		Sleep(100);
		while (win.poll_event(ev))
		{
			//if (ev.type != ksn::event_type_t::mouse_move) ev.dump();

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
