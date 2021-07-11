
int main()
{



}








//#include <ksn/image.hpp>
//#include <ksn/crc.hpp>
//#include <ksn/window.hpp>
//#include <ksn/stuff.hpp>
//
//#include <windows.h>
//
//#pragma comment(lib, "libksn_image.lib")
//#pragma comment(lib, "libksn_crc.lib")
//#pragma comment(lib, "libksn_window.lib")
//#pragma comment(lib, "libksn_time.lib")
//#pragma comment(lib, "libksn_stuff.lib")
//
//#pragma comment(lib, "zlibstatic.lib")
//
//#include <filesystem>


//
//
//void clear(ksn::window_t& win, ksn::color_rgb_t color = {})
//{
//	auto height = win.get_height();
//	auto width = win.get_width();
//	
//	static thread_local uint8_t* data = nullptr;
//	static thread_local int size = 0;
//	int current_size = height * width * 3;
//
//	if (current_size > size)
//	{
//		free(data);
//		if ((data = (uint8_t*)malloc(current_size)) == nullptr) abort();
//		size = current_size;
//	}
//
//	for (size_t i = width * (size_t)height; i --> 0;)
//	{
//		data[3 * i + 0] = color.b;
//		data[3 * i + 1] = color.g;
//		data[3 * i + 2] = color.r;
//	}
//	win.draw_pixels_bgr_front(data, 0, 0, width, height);
//}
//
//
//int main1()
//{
//	constexpr static uint32_t clear_color = 0x003000;
//
//	auto directory = std::filesystem::directory_iterator("D:/png");
//	auto iter = begin(directory);
//	while (iter != end(directory) && !iter->is_regular_file()) ++iter;
//	if (iter == end(directory))
//	{
//		MessageBoxA(GetConsoleWindow(), "No files in D:/png/", "", MB_ICONERROR);
//		return 1;
//	}
//
//	ksn::image_bgra_t img;
//	ksn::window_t win;
//
//	win.open(800, 600);
//	clear(win, clear_color);
//
//	auto do_picture = [&]
//	{
//		while (!iter->is_regular_file()) ++iter;
//
//		int ok = img.load_from_file(iter->path().string().data());
//		if (ok == ksn::image_bgra_t::load_result::ok)
//		{
//			clear(win, clear_color);
//			win.draw_pixels_bgra_front(img.m_data.data(), 0, 0, img.width, img.height);
//		}
//		else
//		{
//			MessageBoxA(win.window_native_handle(), iter->path().string().data(), "Failed to load file", MB_ICONERROR);
//		}
//
//		if (++iter == end(directory))
//			iter = begin(directory = std::filesystem::directory_iterator("D:/png"));
//	};
//
//	win.set_framerate(20);
//
//	while (win.is_open())
//	{
//
//
//		win.tick();
//
//
//		ksn::event_t ev;
//		while (win.poll_event(ev))
//		{
//			switch (ev.type)
//			{
//			case ksn::event_type_t::keyboard_press:
//				switch (ev.keyboard_button_data.button)
//				{
//				case ksn::keyboard_button_t::esc:
//					win.close();
//					break;
//
//				case ksn::keyboard_button_t::space:
//					do_picture();
//					break;
//
//				default:
//					break;
//				}
//				break;
//
//
//			case ksn::event_type_t::close:
//				win.close();
//				break;
//
//			default:
//				break;
//			}
//		}
//	}
//
//	return 0;
//}
