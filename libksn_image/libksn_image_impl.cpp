
/*

TODO:
	Fix minor png load glitch
	Implement interlacing
	Clean up code
	Implement BMP handling
	For future:
		Probably implement JPEG handling

*/


#include <ksn/image.hpp>
#include <ksn/crc.hpp>
#include <ksn/stuff.hpp>
#include <ksn/metapr.hpp>

#include <zlib.h>

#include <stdio.h>


#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 4996 6054)
#endif



_KSN_BEGIN



image_bgra_t::image_bgra_t() noexcept
	: width(0), height(0)
{
}

image_bgra_t::image_bgra_t(image_bgra_t&& other) noexcept
{
	this->swap(other);
}

image_bgra_t& image_bgra_t::operator=(image_bgra_t&& rhs) noexcept
{
	this->swap(rhs);
	return *this;
}

void image_bgra_t::clear() noexcept
{
	this->width = this->height = 0;
	this->m_data.clear();
}

void image_bgra_t::swap(image_bgra_t& other) noexcept
{
	this->m_data.swap(other.m_data);
	std::swap(this->width, other.width);
	std::swap(this->height, other.height);
}


image_bgra_t::load_result_t try_load_png(FILE* fd, image_bgra_t* data);
image_bgra_t::load_result_t try_load_bmp(FILE* fd, image_bgra_t* data);

image_bgra_t::load_result_t image_bgra_t::load_from_file(const char* fname) noexcept
{
	this->clear();

	try
	{
		FILE* f = fopen(fname, "rb");
		if (!f) return load_result::file_unavailable;

		struct __on_return_t
		{
			FILE* f;
			~__on_return_t() { fclose(f); }
		} __on_return_t{ f };

		load_result_t(*load_handlers[])(FILE*, image_bgra_t*) =
		{
			try_load_png
		};

		for (auto handler : load_handlers)
		{
			auto result = handler(f, this);
			if (result != load_result::file_invalid)
				return result;
		}

		return load_result::file_invalid;
	}
	catch (const std::bad_alloc&)
	{
		return load_result::out_of_memory;
	}
	catch (...)
	{
		return load_result::internal_error;
	}
}



image_bgra_t image_bgra_t::scaled(uint16_t new_width, uint16_t new_height) const
{
	if (new_width == 0 || new_height == 0)
		return {};

	image_bgra_t result;
	result.m_data.resize(new_height * (size_t)new_width);
	result.width = new_width;
	result.height = new_height;

	float ratio_x = float(this->width) / new_width;
	float ratio_y = float(this->height) / new_height;

	auto avg = [&]
	(float y, float x) -> ksn::color_bgra_t
	{
		float x1 = x;
		float b = 0, g = 0, r = 0, a = 0;

		for (float bound_y = std::min(y + ratio_y, (float)this->height); y < bound_y;)
		{
			float dy = std::min(1 - (y - (int)y), bound_y - y);

			for (float bound_x = std::min(x + ratio_x, (float)this->width); x < bound_x;)
			{
				float dx = std::min(1 - (x - (int)x), bound_x - x);

				ksn::color_bgra_t pixel = this->m_data[(size_t)x + this->width * (size_t)y];

				float ds = dx * dy;
				b += pixel.b * ds;
				g += pixel.g * ds;
				r += pixel.r * ds;
				a += pixel.a * ds;

				x += dx;
			}

			y += dy;
			x = x1;
		}

		float S = ratio_x * ratio_y;
		b /= S;
		g /= S;
		r /= S;
		a /= S;
		return ksn::color_bgra_t((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
	};

	size_t downscaled_offset = (size_t)new_width * new_height;

	for (size_t i = result.height; i-- > 0;)
	{
		for (size_t j = result.width; j-- > 0;)
		{
			result.m_data[--downscaled_offset] = avg(i * ratio_y, j * ratio_x);
		}
	}

	return result;
}

void image_bgra_t::scale(uint16_t new_width, uint16_t new_height)
{
	*this = this->scaled(new_width, new_height);
}





bool check_signature(FILE* f, const char* data, size_t size)
{
	const uint8_t* p = (const uint8_t*)data;
	while (size --> 0)
	{
		if (fgetc(f) != *p++)
			return false;
	}
	return true;
}



#define assert_corrupted(expr) if (!(expr)) { return image_bgra_t::load_result::file_corrupted; } else ksn::nop()
#define assert_storage(storage, minimum_size) if (storage.size() < minimum_size) { storage.resize(minimum_size); } else ksn::nop()
#define update_crc_with_var(crc, data) if constexpr (true) { crc = ksn::crc32_update(&(data), sizeof(data), crc); } else ksn::nop()
#define read_var(fd, var, crc) if constexpr (true) { assert_corrupted(file_read_bin_data(fd, var)); update_crc_with_var(crc, var); bswap(var); } else ksn::nop()
#define read_var_nocrc(fd, var) if constexpr (true) { assert_corrupted(file_read_bin_data(fd, var)); bswap(var); } else ksn::nop()


template<class T>
struct is_bool_const
	: std::false_type
{
};
template<bool b>
struct is_bool_const<std::bool_constant<b>>
	: std::true_type
{
};

template<class T>
static constexpr bool is_bool_const_v = is_bool_const<T>::value;



template<size_t _channels, size_t _bits>
struct png_recompress_data
{
	static constexpr size_t channels = _channels;
	static constexpr size_t bits = _bits;
};



image_bgra_t::load_result_t try_load_png(FILE* fd, image_bgra_t* image)
{
	if (!check_signature(fd, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8))
		return image_bgra_t::load_result::file_invalid;


	std::vector<uint8_t> png_compressed_storage;
	std::vector<uint8_t> png_decompressed_storage;

	auto& data = image->m_data;

	uint32_t& width = image->width, & height = image->height;
	uint8_t bit_depth, color_type, compression_method, filter_method, interlace_method;
	uint8_t filter_type, bpp;
	bool alpha_channel_present;
	size_t pic_size;
	uint8_t channels;

	static color_bgr_t palette[256];
	uint8_t palette_size;

	int dtemp;


	//search for IHDR
	while (true)
	{
		uint32_t header_length;
		read_var_nocrc(fd, header_length);

		char chunk_name[4];
		assert_corrupted(fread(chunk_name, 1, 4, fd) == 4);

		//If a current chunk is not IHDR
		if (header_length != 13 || strncmp(chunk_name, "IHDR", 4) != 0)
		{
			//Seek to the next chunk
			fseek(fd, header_length + 4, SEEK_CUR);
			continue;
		}


		//Exract all the data from the header chunk

		uint32_t header_crc_expected = ksn::crc32("IHDR", 4);

		read_var(fd, width, header_crc_expected);
		read_var(fd, height, header_crc_expected);
		assert_corrupted(width && height);
		data.resize(pic_size = size_t(width) * height);

		read_var(fd, bit_depth, header_crc_expected);
		assert_corrupted(bit_depth <= 16 && bit_depth > 0 && (bit_depth & (bit_depth - 1)) == 0);
		/* ^^^ assert(bit_depth is a power of two in[1; 16]) ^ ^^ */

		read_var(fd, color_type, header_crc_expected);
		assert_corrupted(color_type <= 6);
		
		static constexpr uint8_t channels_array[7] = { 1, 0, 3, 1, 2, 0, 4 };
		channels = channels_array[color_type];
		assert_corrupted(channels != 0);

		static constexpr uint8_t bpps[7][17] = //combinations of color type and bit depth giving bytes per pixel in the decompressed input
		{
			{ 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 6 },
			{ 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 4 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 8 },
		};

		assert_corrupted((bpp = bpps[color_type][bit_depth]) != 0);

		alpha_channel_present = color_type & 4;
		color_type &= 3;
		
		read_var(fd, compression_method, header_crc_expected);
		read_var(fd, filter_method, header_crc_expected);
		read_var(fd, interlace_method, header_crc_expected);

		//It is already kinda painful to implement ones that are currently present
		if (compression_method || filter_method || interlace_method) return image_bgra_t::load_result::unimplemented;

		uint32_t header_crc;
		read_var_nocrc(fd, header_crc);
		assert_corrupted(header_crc == header_crc_expected);

		break;
	}



	assert_storage(png_compressed_storage, pic_size * bpp + height);
	assert_storage(png_decompressed_storage, pic_size * bpp + height);



	z_stream data_stream{};
	dtemp = inflateInit(&data_stream);
	switch (dtemp)
	{
	case Z_OK: break;
	case Z_MEM_ERROR: return image_bgra_t::load_result::out_of_memory;
	default: return image_bgra_t::load_result::internal_error;
	}

	struct _destruct_sentry_t
	{
		z_stream* watchee_zstream = nullptr;

		_destruct_sentry_t(z_stream* watchee_zstream)
			: watchee_zstream(watchee_zstream)
		{
		}

		~_destruct_sentry_t()
		{
			//Safety first
			ksn_dynamic_assert(this->watchee_zstream != nullptr, "");

			inflateEnd(this->watchee_zstream);
		}
	} _destruct_sentry(&data_stream);



	//search for PLTE for indexed-color images
	if (color_type == 3)
	{
		while (true)
		{
			uint32_t chunk_length;
			uint32_t chunk_crc_expected;

			read_var_nocrc(fd, chunk_length);

			char chunk_name[4];
			assert_corrupted(fread(chunk_name, 1, 4, fd) == 4);
			if (strncmp(chunk_name, "PLTE", 4) != 0)
			{
				fseek(fd, chunk_length + 4, SEEK_CUR);
				continue;
			}

			assert_corrupted((chunk_length % 3) == 0 && chunk_length >= 3 && chunk_length <= 768);

			palette_size = chunk_length / 3 - 1; //Contains max palette index

			assert_corrupted(fread(palette, 1, chunk_length, fd) == chunk_length);

			chunk_crc_expected = ksn::crc32("PLTE", 4);
			chunk_crc_expected = ksn::crc32_update(palette, chunk_length, chunk_crc_expected);

			uint32_t chunk_crc;
			read_var_nocrc(fd, chunk_crc);
			assert_corrupted(chunk_crc == chunk_crc_expected);

			chunk_length /= 3;

			while (chunk_length --> 0) bswap(palette[chunk_length]);

			break;
		}
	}



	//search for IDAT and IEND

	while (true)
	{
		uint32_t chunk_crc_expected;

		uint32_t chunk_length;
		read_var_nocrc(fd, chunk_length);

		char chunk_name[4];
		assert_corrupted(fread(chunk_name, 1, 4, fd) == 4);
		if (chunk_length == 0 && strncmp(chunk_name, "IEND", 4) == 0) break;
		if (strncmp(chunk_name, "IDAT", 4) != 0)
		{
			fseek(fd, chunk_length + 4, SEEK_CUR);
			continue;
		}

		//Extact the data from all the IDAT chunks in a cumulative manner

		assert_storage(png_compressed_storage, (size_t)data_stream.avail_in + chunk_length);
		uint8_t* pbuf = png_compressed_storage.data() + data_stream.avail_in;

		assert_corrupted(fread(pbuf, 1, chunk_length, fd) == chunk_length);
		data_stream.avail_in += chunk_length;

		chunk_crc_expected = ksn::crc32("IDAT", 4);
		chunk_crc_expected = ksn::crc32_update(pbuf, chunk_length, chunk_crc_expected);

		uint32_t chunk_crc;
		read_var_nocrc(fd, chunk_crc);
		assert_corrupted(chunk_crc_expected == chunk_crc);
	}



	//Decompress input data

	data_stream.next_in = png_compressed_storage.data();
	data_stream.avail_out = (uint32_t)png_decompressed_storage.size();
	data_stream.next_out = png_decompressed_storage.data();
	//data_stream.avail_in is set during IDAT chunk(s) processing

	dtemp = inflate(&data_stream, Z_FINISH);
	png_compressed_storage.clear();
	png_compressed_storage.shrink_to_fit();
	assert_corrupted(dtemp == Z_STREAM_END);



	//Unfilter
	
	uint8_t* pin = png_decompressed_storage.data();
	size_t byte_width = size_t(width) * channels * bit_depth;
	byte_width = byte_width / 8 + bool(byte_width % 8);

	if (filter_method == 0)
	{
		auto _get_unfixed = [&]
		(size_t pos) -> uint8_t&
		{
			pos += pos / byte_width;
			return png_decompressed_storage[pos];
		};

		auto get = [&]
		(size_t pos) -> uint8_t&
		{
			pos += pos / byte_width + 1;
			return png_decompressed_storage[pos];
		};

		auto raw = [&]
		(size_t pos) -> uint8_t
		{
			if (*std::launder(&pos) > SIZE_MAX / 2) return 0;
			return get(pos);
		};

		auto raw_j = [&]
		(size_t pos, size_t j) -> uint8_t
		{
			if (j == 0) return 0;
			return raw(pos);
		};

		auto prior = [&]
		(size_t pos) -> uint8_t
		{
			return raw(pos - byte_width);
		};

		auto prior_j = [&]
		(size_t pos, size_t j) -> uint8_t
		{
			return raw_j(pos - byte_width, j);
		};

		auto paeth_predictor = [] //man wtf is this function //oh ok i get it
		(int16_t a, int16_t b, int16_t c) -> uint8_t
		{
			int16_t p = a + b - c;
			int16_t pa = p >= a ? p - a : a - p;
			int16_t pb = p >= b ? p - b : b - p;
			int16_t pc = p >= c ? p - c : c - p;
			if (pa <= pb && pa <= pc) return (uint8_t)a;
			if (pb <= pc) return (uint8_t)b;
			return (uint8_t)c;
		};


		size_t row_offset = 0;

		auto worker = [&]
		(auto updater)
		{
			size_t pos = row_offset;
			for (size_t j = 0; j < byte_width; ++j, ++pos )
			{
				get(pos) = updater(pos, j);
			}
		};
		
		for (size_t i = 0; i < height; ++i )
		{
			switch (filter_type = _get_unfixed(row_offset))
			{
			case 0:
				break;
			case 1:
				worker([&](size_t pos, size_t j) { return raw(pos) + raw_j(pos - bpp, j); });
				break;
			case 2:
				worker([&](size_t pos, size_t j) { return raw(pos) + prior(pos); });
				break;
			case 3:
				worker([&](size_t pos, size_t j) { return raw(pos) + (prior(pos) + raw_j(pos - bpp, j)) / 2; });
				break;
			case 4:
				worker([&](size_t pos, size_t j) { return raw(pos) + paeth_predictor(raw_j(pos - bpp, j), prior(pos), prior_j(pos - bpp, j)); });
				break;
			default:
				assert_corrupted(false);
			}

			row_offset += byte_width;
		}
	}


	//Remove filter type bytes
	{
		std::vector<uint8_t> png_unfiltered_data(pic_size * bpp);

		const uint8_t* p1 = png_decompressed_storage.data() + 1;
		uint8_t* p2 = png_unfiltered_data.data();
		for (size_t i = height; i-- > 0;)
		{
			memcpy(p2, p1, byte_width);
			p1 += byte_width + 1;
			p2 += byte_width;
		}

		png_decompressed_storage = std::move(png_unfiltered_data);
	}




	//If the bit depth per sample was less than 8 bits, expand it into bytes

	if (bit_depth < 8 && color_type != 3) //remember not to do this when a palette is present
	{
		std::vector<uint8_t> png_recompressed_storage;
		assert_storage(png_recompressed_storage, png_decompressed_storage.size());

		uint8_t* pin = png_decompressed_storage.data();
		uint8_t* pout = png_recompressed_storage.data();

		auto worker = [&]
		(auto options)
		{
			static constexpr size_t channels = decltype(options)::channels;
			static constexpr size_t bits = decltype(options)::bits;
			static constexpr uint8_t bit_offset_initial = 8 - bits;
			static constexpr uint8_t bits_initial = ((1 << bits) - 1) << bit_offset_initial;

			static_assert(bits == 1 || bits == 2 || bits == 4);

			for (size_t i = height; i --> 0;)
			{
				uint8_t current_bits = bits_initial;
				int8_t current_bit_offset = bit_offset_initial;

				for (size_t j = width; j --> 0;)
				{
					for (size_t k = channels; k --> 0;)
					{
						uint8_t byte = (*pin & current_bits) >> current_bit_offset;
						*pout++ = byte;

						current_bits = current_bits >> bits | current_bits << (8 - bits);
						current_bit_offset -= bits;
						if (current_bit_offset < 0) current_bit_offset += 8;
						if (current_bit_offset == bit_offset_initial) pin++;
					}
				}

				if (current_bit_offset != bit_offset_initial) pin++;
			}
		};

		switch (color_type | ((int)alpha_channel_present << 2))
		{
		case 0:
			switch (bit_depth)
			{
			case 1: worker(png_recompress_data<1, 1>()); break;
			case 2: worker(png_recompress_data<1, 2>()); break;
			case 4: worker(png_recompress_data<1, 4>()); break;
			default: return image_bgra_t::load_result::internal_error;
			};
			break;

		case 2:
			switch (bit_depth)
			{
			case 1: worker(png_recompress_data<3, 1>()); break;
			case 2: worker(png_recompress_data<3, 2>()); break;
			case 4: worker(png_recompress_data<3, 4>()); break;
			default: return image_bgra_t::load_result::internal_error;
			};
			break;

		case 4:
			switch (bit_depth)
			{
			case 1: worker(png_recompress_data<2, 1>()); break;
			case 2: worker(png_recompress_data<2, 2>()); break;
			case 4: worker(png_recompress_data<2, 4>()); break;
			default: return image_bgra_t::load_result::internal_error;
			};
			break;

		case 6:
			switch (bit_depth)
			{
			case 1: worker(png_recompress_data<4, 1>()); break;
			case 2: worker(png_recompress_data<4, 2>()); break;
			case 4: worker(png_recompress_data<4, 4>()); break;
			default: return image_bgra_t::load_result::internal_error;
			};
			break;

		default:
			return image_bgra_t::load_result::internal_error;
		}

		png_decompressed_storage = std::move(png_recompressed_storage);
	}
	else if (bit_depth == 16) //convert to 8 bits per sample
	{
		uint8_t* pin = png_decompressed_storage.data();
		uint8_t* pout = png_decompressed_storage.data();

		auto worker = [&]
		(auto options)
		{
			static constexpr size_t channels = decltype(options)::channels;

			for (size_t i = height; i --> 0;)
			{
				*pin++ = *pout++; //Filter byte

				for (size_t j = width; j --> 0;)
				{
					for (size_t k = channels; k --> 0;)
					{
						uint16_t word = *(uint16_t*)pin;
						pin += 2;
						*pout++ = word / 257;
					}
				}
			}

			png_decompressed_storage.resize(pout - png_decompressed_storage.data());
		};
	}
	


	//convert to 32-bit rgba

	pin = png_decompressed_storage.data();
	uint16_t* pin16 = (uint16_t*)pin;
	auto* pout = data.data();

	auto graycsale_converter = [&]
	(auto* pin, auto with_alpha)
	{
		float max = float(1 << bit_depth) - 1;
		for (size_t i = 0; i < pic_size; ++i)
		{
			uint8_t value = uint8_t(*pin++ / max * 255 + 0.5f);
			if constexpr (decltype(with_alpha)::value)
				*pout++ = color_bgra_t(value, value, value, uint8_t(*pin++ / max * 255 + 0.5f));
			else
				*pout++ = color_bgra_t(value, value, value, 255);
		}
	};

	auto truecolor_converter = [&]
	(auto* pin, auto with_alpha)
	{
		float max = float(1 << bit_depth) - 1;
		for (size_t i = 0; i < pic_size; ++i)
		{
			uint8_t valueR = uint8_t(*pin++ / max * 255 + 0.5f);
			uint8_t valueG = uint8_t(*pin++ / max * 255 + 0.5f);
			uint8_t valueB = uint8_t(*pin++ / max * 255 + 0.5f);
			if constexpr (decltype(with_alpha)::value)
				*pout++ = color_bgra_t(valueR, valueG, valueB, uint8_t(*pin++ / max * 255 + 0.5f));
			else
				*pout++ = color_bgra_t(valueR, valueG, valueB, 255);
		}
	};

	switch (color_type)
	{
	case 0:
		if (bit_depth <= 8) 
			alpha_channel_present ? graycsale_converter(pin, std::true_type()) : graycsale_converter(pin, std::false_type());
		else
			alpha_channel_present ? graycsale_converter((uint16_t*)pin, std::true_type()) : graycsale_converter((uint16_t*)pin, std::false_type());
		break;

	case 2:
		if (bit_depth <= 8)
			alpha_channel_present ? truecolor_converter(pin, std::true_type()) : truecolor_converter(pin, std::false_type());
		else
			alpha_channel_present ? truecolor_converter((uint16_t*)pin, std::true_type()) : truecolor_converter((uint16_t*)pin, std::false_type());
		break;

	case 3:
	{
		const uint8_t bit_offset_initial = 8 - bit_depth;

		for (size_t i = height; i --> 0;)
		{
			uint8_t current_bits = ((1 << bit_depth) - 1) << bit_offset_initial;
			int8_t current_bit_offset = bit_offset_initial;

			for (size_t j = width; j --> 0;)
			{
				uint8_t index = (*pin & current_bits) >> current_bit_offset;
				*pout++ = palette[index];

				current_bits = current_bits >> bit_depth | current_bits << (8 - bit_depth);
				current_bit_offset -= bit_depth;
				if (current_bit_offset < 0) current_bit_offset += 8;
				if (current_bit_offset == bit_offset_initial) pin++;
			}

			if (current_bit_offset != bit_offset_initial) pin++;
		}
	}
		break;

	default: //unreachable
		break;
	}

	return image_bgra_t::load_result::ok;
}


_KSN_END
