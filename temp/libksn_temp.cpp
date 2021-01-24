
#include <ksn/graphics_engine_window.hpp>

#include <Windows.h>
#include <GL/GL.h>

#include <exception>

#include <stdio.h>


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

#pragma comment(lib, "libksn_graphics_engine.lib")



int main1()
{
	double x = 1;
	uint64_t N = 1000000000;

	for (size_t i = 1; i < N; i++)
	{
		x *= (1 - i / 281474976710656.0);
	}

	printf("%.17lf", x);
	return 0;
}

int main()
{

	ksn::window_t::context_settings settings;
	settings.ogl_version_major = 3;
	settings.ogl_version_minor = 1;
	settings.ogl_compatibility_profile = true;
	
	ksn::window_t window(300, 300, "", settings);
	const GLubyte* p;
	p = glGetString(GL_EXTENSIONS);
	p = glGetString(GL_RENDERER);
	p = glGetString(GL_VENDOR);
	p = glGetString(GL_VERSION);
	
	try
	{
		return 0;
	}
	catch (std::exception e)
	{
		printf("%s\n", e.what());
	}
}
