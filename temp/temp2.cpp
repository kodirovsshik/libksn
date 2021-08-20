
#include <ksn/window.hpp>

#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_time.lib")

int main()
{

	ksn::window_t win;
	win.open(200, 200);
	win.set_framerate(60);

	while (true)
	{
		ksn::event_t ev;
		while (win.poll_event(ev))
		{

		}

		win.tick();
	}

	return 0;
}
