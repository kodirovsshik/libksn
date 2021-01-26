
#include <ksn/window.hpp>

#include <Windows.h>
#include <GL/GL.h>

#include <exception>

#include <stdio.h>


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

#pragma comment(lib, "libksn_window.lib")



int main()
{
	SetLastError(0);

	ksn::window_t win;
	if (!win.open(300, 300, "", { 3, 1 }))
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
		printf("Running OpenGL %s on %s", glGetString(GL_VERSION), /*glGetString(GL_VENDOR),*/ glGetString(GL_RENDERER));
	}


	//ksn::window_t::event_t ev;
	while (1)
	{
		MSG msg;
		printf("a");
		while (win.poll_native_event(msg))
		{
			printf("b");
		}
		printf("c");
		Sleep(50);
	}

}
















//int main1()
//{
//	double x = 1;
//	uint64_t N = 1000000000;
//
//	for (size_t i = 1; i < N; i++)
//	{
//		x *= (1 - i / 281474976710656.0);
//	}
//
//	printf("%.17lf", x);
//	return 0;
//}
//
//int main()
//{
//
//	ksn::window_t::context_settings settings;
//	settings.ogl_version_major = 3;
//	settings.ogl_version_minor = 1;
//	settings.ogl_compatibility_profile = true;
//	
//	ksn::window_t window(300, 300, "", settings);
//	const GLubyte* p;
//	p = glGetString(GL_EXTENSIONS);
//	p = glGetString(GL_RENDERER);
//	p = glGetString(GL_VENDOR);
//	p = glGetString(GL_VERSION);
//	
//	try
//	{
//		return 0;
//	}
//	catch (std::exception e)
//	{
//		printf("%s\n", e.what());
//	}
//}
