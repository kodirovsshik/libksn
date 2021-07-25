
#include <ksn/window.hpp>
#include <ksn/time.hpp>

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")

#include <vector>
#include <array>

int main()
{

	static constexpr int X = 600;
	static constexpr int bpp = 3;

	ksn::window_t win;
	win.open(X, X);

	size_t memsize = (size_t)bpp * X * X;
	uint8_t* data = (uint8_t*)malloc(memsize);
	memset(data, 0x80, memsize);

	ksn::stopwatch sw;

	sw.start();
	win.draw_pixels_bgr_front(data, 0, 0, X, X);
	auto dt = sw.stop();

	printf("%06lli\n", dt.as_usec());

	win.set_framerate(20);

	while (true)
	{
		win.discard_all_events();
		win.tick();
	}

	return 0;
}