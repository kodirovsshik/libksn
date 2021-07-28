
#ifndef _KSN_LOGGER_HPP_
#define _KSN_LOGGER_HPP_



#include <ksn/ksn.hpp>
#include <ksn/stuff.hpp>

#include <vector>
#include <variant>
#include <iostream>

#include <stdarg.h>



_KSN_BEGIN


class logger
{
public:

	virtual void log() noexcept = 0;
};

class file_logger
{
public:

	std::vector<FILE*> m_files;
	std::vector<bool> m_ownerships;


	static constexpr int add_ok = 0;
	static constexpr int add_open_fail = 1;
	static constexpr int add_bad_alloc = 2;
	static constexpr int add_invalid_file = 3;


	inline int add_file(FILE* f, bool take_ownership = false) noexcept;
	template<class char1_t, class char2_t>
	inline int add_file(const char1_t* file_name, const char2_t* open_mode = "w") noexcept;


	inline void log(const char* fmt, ...) noexcept;
	inline void log(const wchar_t* fmt, ...) noexcept;
	inline void log(const char* fmt, va_list) noexcept;
	inline void log(const wchar_t* fmt, va_list) noexcept;


	inline ~file_logger() noexcept;
};


_KSN_END





///////////////////////////////////////////////////
// 
//				Implementation
// 
///////////////////////////////////////////////////





_KSN_BEGIN


int file_logger::add_file(FILE* fd, bool take_ownership) noexcept
{
	fpos_t unused;
	if (!fd || fgetpos(fd, &unused) != 0)
		return file_logger::add_invalid_file;

	try
	{
		this->m_files.push_back(fd);
	}
	catch (...)
	{
		return file_logger::add_bad_alloc;
	}

	try
	{
		this->m_ownerships.push_back(take_ownership);
	}
	catch (...)
	{
		this->m_files.pop_back();
		return file_logger::add_bad_alloc;
	}

	return file_logger::add_ok;
}

template<class char1_t, class char2_t>
int file_logger::add_file(const char1_t* file_name, const char2_t* open_mode) noexcept
{
	return this->add_file(ksn::fopen(file_name, open_mode), true);
}



void file_logger::log(const char* fmt, ...) noexcept
{
	va_list ap;
	va_start(ap, fmt);

	this->log(fmt, ap);

	va_end(ap);
}
void file_logger::log(const wchar_t* fmt, ...) noexcept
{
	va_list ap;
	va_start(ap, fmt);

	this->log(fmt, ap);

	va_end(ap);
}
void file_logger::log(const char* fmt, va_list ap) noexcept
{
	for (FILE* fd : this->m_files)
		vfprintf(fd, fmt, ap);
}
void file_logger::log(const wchar_t* fmt, va_list ap) noexcept
{
	for (FILE* fd : this->m_files)
		vfwprintf(fd, fmt, ap);
}


file_logger::~file_logger() noexcept
{
	for (size_t i = 0; i < this->m_files.size(); ++i)
	{
		if (this->m_ownerships[i])
			fclose(this->m_files[i]);
	}
}

_KSN_END

#endif //!_KSN_LOGGER_HPP_
