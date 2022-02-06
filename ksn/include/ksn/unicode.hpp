
#ifndef _KSN_UNICODE_HPP_
#define _KSN_UNICODE_HPP_


#include <ksn/ksn.hpp>
#include <ksn/stuff.hpp>

#include <wchar.h>

#include <type_traits>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif



_KSN_BEGIN


struct unicode_character
{
	char32_t m_value;

	//Reads the value into current character, an encoding format is deduced from the template parameter
	//Return value:
	//> 0 means count of characters read
	//== 0 means NULL character read
	//< 0 means not a valid unicode character or the size is > 4
	template<class char_t>
	int read(const char_t*) noexcept;


	//Reads the value into current character, an encoding format is deduced from the template parameter
	//Return value:
	//> 0 means count of characters read
	//== 0 means NULL character read
	//< 0 means not a valid unicode character or the size is > 4
	//Unckecked means:
	//1) Unpaired surrogates are valid in UTF-16
	//2) No checks for strong 10 match in 10xxxxxx bit pattern when decoding UTF-8
	template<class char_t>
	int read_unchecked(const char_t*) noexcept;


	//Converts the current character to an encoding format is deduced from the template parameter
	//Return value:
	//> 0 means number of characters written (not the case if NULL character written)
	//== 0 means NULL character is written
	//< 0 means the object is an invalid character and nothing was stored to the pointer
	//If the pointer is nullptr, then the return is the amount of bytes the character will take in a detected encoding
	template<class char_t>
	int store(char_t*) noexcept;

	//Converts the current character to an encoding format is deduced from the template parameter
	//Return value:
	//> 0 means number of characters written (not the case if NULL character written)
	//== 0 means NULL character is written
	//< 0 means the object is an invalid character and nothing was stored to the pointer
	//If the pointer is nullptr, then the return is the amount of bytes the character will take in a detected encoding
	//Unckecked means code points [0xD800; 0xDFFF] are coded as a single UTF-16 unpaired surrogate
	template<class char_t>
	int store_unchecked(char_t*) noexcept;
};


template<typename out_str_t, bool strict = true, class in_str_t = std::string>
out_str_t unicode_string_convert(const in_str_t& str);


_KSN_END








_KSN_BEGIN


_KSN_DETAIL_BEGIN


template<bool check, class char_t>
int unicode_le_to_utf32le(const char_t* p, char32_t& c)
{
	if constexpr (std::is_same_v<char8_t, char_t> || (std::is_same_v<wchar_t, char_t> && sizeof(wchar_t) == sizeof(char8_t)) ||
		std::is_same_v<char, char_t> || std::is_same_v<signed char, char_t> || std::is_same_v<unsigned char, char_t>)
	{
		if (*p == 0)
		{
			c = 0;
			return 0;
		}

		if ((int8_t)*p < 0)
		{
			size_t n = -1;

			uint8_t byte = (uint8_t)*p;
			while ((int8_t)byte < 0)
			{
				n++;
				byte <<= 1;
			}
			
			char32_t c32 = byte >> (n + 1);

			for (size_t i = n; i --> 0;)
			{
				byte = *++p;

				if constexpr (check)
					if ((byte & 0b11000000) == 0)
						return -1;

				c32 = (c32 << 6) | (byte & 63);
			}
			
			c = c32;
			return c == 0 ? 0 : (int)n + 1;
		}
		else
		{
			c = *p;
			return 1;
		}
	}

	else if constexpr (std::is_same_v<char16_t, char_t> || (std::is_same_v<wchar_t, char_t> && sizeof(wchar_t) == 2))
	{
		char16_t wc[2];
		wc[0] = p[0];

		if ((wc[0] & 0b1111110000000000) == 0xD800)
		{
			wc[1] = p[1];
			if constexpr (check)
			{
				if ((wc[1] & 0b1111110000000000) != 0xDC00)
					return -1;
			}
			else
			{
				if ((wc[1] & 0b1111110000000000) != 0xDC00)
				{
					c = wc[0] & 0b0000001111111111;
					return 1;
				}
			}
			c = (((wc[0] & 1023) << 10) | (wc[1] & 1023)) + 0x10000;
			return c == 0 ? 0 : 2;
		}
		else
		{
			c = *p;
			return c == 0 ? 0 : 1;
		}
	}

	else if constexpr (std::is_same_v<char32_t, char_t> || (std::is_same_v<wchar_t, char_t> && sizeof(wchar_t) == 4))
	{
		c = *p;
		return c == 0 ? 0 : 1;
	}

	else
	{
		_ct_assert<false>();
	}

	return -1;
}

template<bool check, class char_t>
int utf32le_to_unicode_le(char32_t c, char_t* p)
{
	if (c == 0)
	{
		if (p) *p = 0;
		return 0;
	}

	if constexpr (std::is_same_v<char8_t, char_t> || (std::is_same_v<wchar_t, char_t> && sizeof(wchar_t) == 1) ||
		std::is_same_v<char, char_t> || std::is_same_v<signed char, char_t> || std::is_same_v<unsigned char, char_t>)
	{
		if ((uint32_t)c <= 0x7F)
		{
			if (p) *p = (char_t)c;
			return 1;
		}
		else if ((uint32_t)c <= 0x7FF)
		{
			if (p)
			{
				*p++ = 0b11000000 | ((c >> 6) & 0b00011111);
				*p = 0b10000000 | (c & 0b00111111);
			}
			return 2;
		}
		else if ((uint32_t)c <= 0xFFFF)
		{
			if (p)
			{
				*p++ = 0b11100000 | ((c >> 12) & 0b00001111);
				*p++ = 0b10000000 | ((c >> 6) & 0b00111111);
				*p = 0b10000000 | (c & 0b00111111);
			}
			return 3;
		}
		else if ((uint32_t)c <= 0x1FFFFF)
		{
			if (p)
			{
				*p++ = 0b11110000 | ((c >> 18) & 0b00000111);
				*p++ = 0b10000000 | ((c >> 12) & 0b00111111);
				*p++ = 0b10000000 | ((c >> 6) & 0b00111111);
				*p = 0b10000000 | (c & 0b00111111);
			}
			return 4;
		}
		else if ((uint32_t)c <= 0x3FFFFFF)
		{
			if (p)
			{
				*p++ = 0b11111000 | ((c >> 24) & 0b00000011);
				*p++ = 0b10000000 | ((c >> 18) & 0b00111111);
				*p++ = 0b10000000 | ((c >> 12) & 0b00111111);
				*p++ = 0b10000000 | ((c >> 6) & 0b00111111);
				*p = 0b10000000 | (c & 0b00111111);
			}
			return 5;
		}
		else if ((uint32_t)c <= 0x7FFFFFFF)
		{
			if (p)
			{
				*p++ = 0b11111100 | ((c >> 30) & 0b00000001);
				*p++ = 0b10000000 | ((c >> 24) & 0b00111111);
				*p++ = 0b10000000 | ((c >> 18) & 0b00111111);
				*p++ = 0b10000000 | ((c >> 12) & 0b00111111);
				*p++ = 0b10000000 | ((c >> 6) & 0b00111111);
				*p = 0b10000000 | (c & 0b00111111);
			}
			return 6;
		}
		else
		{
			return -1;
		}
	}

	else if constexpr (std::is_same_v<char_t, char16_t> || (std::is_same_v<char_t, wchar_t> && sizeof(wchar_t) == sizeof(char16_t)))
	{
		if (c > 0x10FFFF)
			return -1;

		if constexpr (check)
		{
			if (c >= 0xD800 && c <= 0xDFFF)
				return -1;
		}

		if (c >= 0x10000)
		{
			c -= 0x10000;

			if (p)
			{
				*p++ = 0b1101100000000000 | ((c >> 10) & 0b0000001111111111);
				*p = 0b1101110000000000 | (c & 0b0000001111111111);
			}

			return 2;
		}
		else
		{
			if (p) *p = (char_t)c;
			return 1;
		}
	}

	else if constexpr (std::is_same_v<char_t, char32_t> || (std::is_same_v<char_t, wchar_t> && sizeof(wchar_t) == sizeof(char32_t)))
	{
		if (p) *p = c;
		return 1;
	}

	else
	{
		_ct_assert<false>();
	}

	return -1;
}



_KSN_DETAIL_END



template<class char_t>
int unicode_character::read(const char_t* p) noexcept
{
	return detail::unicode_le_to_utf32le<true>(p, this->m_value);
}
template<class char_t>
int unicode_character::read_unchecked(const char_t* p) noexcept
{
	return detail::unicode_le_to_utf32le<false>(p, this->m_value);
}
template<class char_t>
int unicode_character::store(char_t* p) noexcept
{
	return detail::utf32le_to_unicode_le<true>(this->m_value, p);
}
template<class char_t>
int unicode_character::store_unchecked(char_t* p) noexcept
{
	return detail::utf32le_to_unicode_le<true>(this->m_value, p);
}



template<typename out_str_t, bool strict, class in_str_t>
out_str_t unicode_string_convert(const in_str_t& str)
{
	using out_char_t = typename out_str_t::value_type;

	out_str_t result;
	result.reserve(str.size());

	const auto* p = str.c_str();

	while (true)
	{
		char32_t c32;
		int local_result = detail::unicode_le_to_utf32le<strict>(p, c32);
		if (local_result < 0)
			return out_str_t();
		if (local_result == 0) break;

		p += local_result;

		local_result = detail::utf32le_to_unicode_le<strict, out_char_t>(c32, nullptr);
		result.resize(result.size() + local_result, ' ');
		local_result = detail::utf32le_to_unicode_le<strict>(c32, result.data() + result.size() - local_result);
	}

	return result;
}





template<class char1_t, class char2_t>
FILE* fopen(const char1_t* name, const char2_t* mode) noexcept
{
	try
	{
#ifdef _WIN32
#pragma warning(push) //fuck you whoever decided to make C4996 an error
#pragma warning(disable : 4996)
		return _wfopen(
			ksn::unicode_string_convert<std::wstring>(std::basic_string<char1_t>(name)).c_str(),
			ksn::unicode_string_convert<std::wstring>(std::basic_string<char2_t>(mode)).c_str()
		);
#pragma warning(pop)
#else
		return ::fopen(
			(const char*)ksn::unicode_string_convert<std::u8string>(std::basic_string<char1_t>(name)).c_str(),
			(const char*)ksn::unicode_string_convert<std::u8string>(std::basic_string<char2_t>(mode)).c_str()
		)
#endif
	}
	catch (...)
	{
		return nullptr;
	}
}



_KSN_END


#endif //!_KSN_UNICODE_HPP_
