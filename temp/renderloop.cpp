
#include <complex.h>

#include <ksn/window.hpp>


#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_time.lib")

void user_update_handle()
{
	static int i = 0;
	printf("%i\n", ++i);
}



ksn::window_t win;

void render()
{

}

void poll()
{
	ksn::event_t ev;
	while (win.poll_event(ev))
	{
		switch (ev.type)
		{
		case ksn::event_type_t::close:
			win.close();
			break;
		}
	}
}

void update()
{
	user_update_handle();
	win.tick();
}



void resizemove(const ksn::resizemove_data_t* data)
{
	update();
	render();
}

void mainloop()
{
	while (win.is_open())
	{
		render();
		update();
	}
}

int main()
{

	win.open(600, 600);
	win.set_framerate(10);
	win.set_resizemode_handle(resizemove);

	while (win.is_open())
	{
		poll();
	}

	return 0;
}