
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
	SetLastError(0);
	int _ = glGetError();

	ksn::window_t::context_settings settings{ 3, 1 };
	ksn::window_t window(800, 600);

	if (true)
	{
		printf("\aFailed to open a window\nHow damn old is your videocard?\n\nWINAPI status: %i\nOpenGL status: %i\n", (int)GetLastError(), (int)glGetError());
		return 1;
	}
	else
	{
		printf("OpenGL context data:\n%s\n%s\n%s\n", glGetString(GL_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	}
	
	try
	{
		return 0;
	}
	catch (std::exception e)
	{
		printf("%s\n", e.what());
	}
}


int main1()
{
	double x = 1;
	uint64_t N = 1000000000, c = 0;

	for (size_t i = 1; i < N; i++)
	{
		x *= (1 - i / 281474976710656.0);
		c++;
		if (c == 10000)
		{
			printf("%.17lf", x);
		}
	}

	printf("%.17lf", x);
	return 0;
}
