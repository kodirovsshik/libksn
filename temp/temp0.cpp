
#include <ksn/window.hpp>
#include <ksn/image.hpp>
#include <ksn/stuff.hpp>
#include <ksn/math_common.hpp>
#include <ksn/time.hpp>


#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_image")
#pragma comment(lib, "libksn_time")
#pragma comment(lib, "libksn_crc")
#pragma comment(lib, "libksn_stuff")
#pragma comment(lib, "zlibstatic")



ksn::image_bgra_t downscale4_nearest(const ksn::image_bgra_t& img)
{
	ksn::image_bgra_t result;
	result.m_data.resize(img.height * img.width / 16);
	result.width = img.width / 4;
	result.height = img.height / 4;

	size_t original_offset = 0;
	size_t downscale_offset = 0;

	for (size_t i = img.height / 4; i --> 0;)
	{
		for (size_t j = img.width / 4; j --> 0 ;)
		{
			result.m_data[downscale_offset++] = img.m_data[(original_offset += 4) - 4];
		}
		original_offset += 3 * (size_t)img.width;
	}

	return result;
}

ksn::image_bgra_t downscale4_avg_with_bottom_left(const ksn::image_bgra_t& img)
{
	ksn::image_bgra_t result;
	result.m_data.resize(img.height * img.width / 16);
	result.width = img.width / 4;
	result.height = img.height / 4;

	auto avg = [&]
	(size_t off)
	{
		uint16_t b = 0, g = 0, r = 0, a = 0;
		auto* p = &img.m_data[off];

		for (size_t j = 4; j; --j)
		{
			for (size_t i = 4; i; --i)
			{
				b += p->b; g += p->g; r += p->r; a += p->a;
				++p;
			}
			p += img.width - 4;
		}

		return ksn::color_bgra_t(r >> 4, g >> 4, b >> 4, a >> 4);
	};

	size_t original_offset = 0;
	size_t downscaled_offset = 0;

	for (size_t i = result.height; i --> 0;)
	{
		for (size_t j = result.width; j --> 0;)
		{
			result.m_data[downscaled_offset++] = avg(original_offset);
			original_offset += 4;
		}
		original_offset += size_t(3) * img.width;
	}

	return result;
}




void clear(ksn::window_t& win, ksn::color_bgr_t color = ksn::color_bgr_t(0))
{
	static thread_local std::vector<ksn::color_bgr_t> data;
	size_t required_capacity = (size_t)win.get_client_width() * win.get_client_height();
	if (data.size() < required_capacity) data.resize(required_capacity);

	if (color.r == color.g && color.g == color.b)
		memset(data.data(), color.r, required_capacity * 3);
	else
		std::fill(data.begin(), data.end(), color);

	win.draw_pixels_bgr_front(data.data());
}

int main()
{
	ksn::stopwatch sw;
	sw.start();

	ksn::image_bgra_t img;
	auto x = img.load_from_file("D:/png/1.png");
	printf("Time = %07lli mcs\n", sw.restart().as_usec());

	ksn::window_t win;
	win.open(1000, 700, "", ksn::window_style::close_min_max | ksn::window_style::hidden);
	win.set_framerate(10);
	printf("Time = %07lli mcs\n", sw.restart().as_usec());

	ksn::image_bgra_t downscaled1 = downscale4_nearest(img);
	printf("Time = %07lli mcs\n", sw.restart().as_usec());
	ksn::image_bgra_t downscaled2 = downscale4_avg_with_bottom_left(img);
	printf("Time = %07lli mcs\n", sw.restart().as_usec());
	ksn::image_bgra_t downscaled3 = img.scaled(downscaled2.width, downscaled2.height);
	printf("Time = %07lli mcs\n", sw.restart().as_usec());

	int ok = img.load_from_file("D:/png/1/test.png");
	ksn::image_bgra_t img_wide_parrot = img.scaled(1000, 700);
	//ksn::image_bgra_t* images[] = { &downscaled1, &downscaled2, &downscaled3 };
	ksn::image_bgra_t* images[] = { &img_wide_parrot };
	ksn::image_bgra_t** image = &images[ksn::countof(images) - 1];

	win.show();

	while (win.is_open())
	{


		win.tick();

		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				win.close();
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::escape:
					win.close();
					break;

				case ksn::keyboard_button_t::space:
					if (++image == std::end(images))
						image = &images[0];

					//clear(win);
					win.draw_pixels_bgra_front((*image)->m_data.data(), 0, 0, (*image)->width, (*image)->height);
				}
				break;
			}
		}
	}

	return 0;
}
