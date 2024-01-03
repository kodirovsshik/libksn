
#include <ksn/image.hpp>

#pragma comment(lib, "libksn_stuff")
#pragma comment(lib, "libksn_image")
#pragma comment(lib, "libksn_crc")
#pragma comment(lib, "zlibstatic")

int main()
{
	ksn::image_t<ksn::color_rgba_t> img;
	img.load_from_file("a.img");
}
