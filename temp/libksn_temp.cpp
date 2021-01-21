
#include <ksn/graphics_engine_window.hpp>

#include <Windows.h>
#include <GL/GL.h>

#include <exception>

#include <stdio.h>


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

#pragma comment(lib, "libksn_graphics_engine.lib")



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
