
//#include <ksn/stuff.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/ppvector.hpp>
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

struct object3d
{
	ksn::ppvector<ksn::graphics::vertex3_t> vertexes;
	ksn::ppvector<ksn::graphics::surface_indexed_t> surfaces;

	uint32_t m_handle = uint32_t(-1);

	bool registreate(ksn::graphics::shape_buffer_t& sb, ksn::graphics::error_t* err = nullptr)
	{
		uint32_t off = sb.registrate(vertexes.m_buffer, vertexes.size(), err);
		if (off == -1) return false;

		this->m_handle = sb.registrate(surfaces.m_buffer, surfaces.size(), off, err);
		return this->m_handle != -1;
	}
};

int main()
{
	
	ksn::graphics::shape_buffer_t sb;

	object3d triangle
	{ 
		{
			{ -0.5f, -0.5f, 0},
			{ 0.5f, -0.5f, 0},
			{ 0, 0.5f, 0},
		}, 
		{
			{0, 1, 2}
		} 
	};

	if (!triangle.registreate(sb)) return 1;

	//ksn::graphics::color_t obama_data[6];

	//ksn::graphics::texture_t obama;
	//obama.data = obama_data;
	//obama.w = 3;
	//obama.h = 2;
	//uint32_t obama_handle = sb.registrate(&obama, 1);

	sb.flush();



	//GD bonfire

	static constexpr size_t w = 400;
	static constexpr size_t h = 300;

	ksn::window_t win;
	ksn::window_t::context_settings settings{ 3, 1, 24, false };
	ksn::window_t::context_settings settings11;
	
	auto* pname = "libKSN window system";

	if (win.open(w, h, pname, settings) != ksn::window_t::error::ok)
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

	
	
	void* data = malloc(w * h * 4);
	if (!data) return 3;
	memset(data, 0, w * h * 4);


	ksn::event_t ev;
	while (win.is_open())
	{
		//Main loop






		//Draw

		glClearColor(1, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		//glFinish();
		
		//glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
		win.swap_buffers();



		//Sleep & process events

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
