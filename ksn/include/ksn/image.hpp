
#ifndef _KSN_IMAGE_HPP_
#define _KSN_IMAGE_HPP_



#include <ksn/ksn.hpp>
#include <ksn/color.hpp>

#include <vector>


_KSN_BEGIN


class image_t
{

public:

	std::vector<ksn::color_rgba_t> m_data;
	uint32_t width, height;



	image_t() noexcept;
	image_t(const image_t&) noexcept = default;
	image_t(image_t&&) noexcept;

	~image_t() noexcept = default;

	image_t& operator=(const image_t&) noexcept = default;
	image_t& operator=(image_t&&) noexcept;
	


	using load_result_t = uint8_t;
	struct load_result
	{
		//ok
		constexpr static load_result_t ok = 0;

		//Failed to open/read
		constexpr static load_result_t file_unavailable = 1;

		//Not a valid image file
		constexpr static load_result_t file_invalid = 2;

		//Failed to allocate memory
		constexpr static load_result_t out_of_memory = 3;

		//Integrity check fail or necessary data is missing
		constexpr static load_result_t file_corrupted = 4;

		//File contains something format decoder is not [yet] implemented for
		constexpr static load_result_t unimplemented = 5;



		constexpr static load_result_t internal_error = -1;
	};

	load_result_t load_from_file(const char* fname) noexcept;

	void clear() noexcept;

	void swap(image_t& other) noexcept;
};


_KSN_END



#endif //!_KSN_IMAGE_HPP_
