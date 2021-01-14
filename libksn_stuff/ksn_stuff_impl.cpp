#include <ksn/stuff.hpp>
#include <ksn/x86_instruction_set.hpp>

#include <intrin.h>

#ifdef _KSN_COMPILER_MSVC
#include <Windows.h>
#endif





_KSN_BEGIN



size_t c16len(const char16_t* p)
{
	const char16_t* pbegin = p;
	while (*p)
	{
		++p;
	}

	return p - pbegin;
}

size_t c32len(const char32_t* p)
{
	const char32_t* pbegin = p;
	while (*p)
	{
		++p;
	}

	return p - pbegin;
}





void _memory_dump_t::operator()(const void* _p, size_t bytes, size_t bytes_per_line, flag_t flags, FILE* fd)
{
	uint8_t* p = (uint8_t*)_p, * pe = p + bytes;
	size_t byte_in_line = 0;
	char hex_a = flags & 1 ? 'a' : 'A';
	char buffer[4] = { 0 };
	size_t len = flags & 2 ? 2 : 3;

	while (p != pe)
	{
		uint8_t temp = *p >> 4;
		if (temp >= 10)
		{
			buffer[0] = hex_a + (temp - 10);
		}
		else
		{
			buffer[0] = temp + '0';
		}

		temp = *p & 0xF;
		if (temp >= 10)
		{
			buffer[1] = hex_a + (temp - 10);
		}
		else
		{
			buffer[1] = temp + '0';
		}

		if (++byte_in_line == bytes_per_line) _KSN_UNLIKELY
		{
			byte_in_line = 0;
			buffer[2] = '\n';
			fwrite(buffer, sizeof(char), 3, fd);
			buffer[2] = ' ';
		}
		else _KSN_LIKELY
		{
			fwrite(buffer, sizeof(char), len, fd);
		}

		++p;
	}
}





const void* memnotchr(const void* block, uint8_t value, size_t length)
{
	if (x86_features->avx512_bw)
	{
		//_mm512_cmpeq_epi8_mask gives 1 on equeal
	}
	else if (x86_features->avx2)
	{
		//_mm256_cmpeq_epi8 gives -1 on equal
	}
	else if (x86_features->sse2)
	{
		//_mm_cmpeq_epi8
		size_t local_size = length / 16;
	}
	else
	{

	}

	throw 0;
}





void dynamic_assert(int nonzero, const char* expr, const char* fmt, int line, const char* file, ...)
{
	if (nonzero == 0)
	{
		fprintf(stderr, "\n\tCRITICAL ERROR: ASSERTION FAILED\a\n\nAt %s: %i\n%s\n\n", file, line, expr);
		
		va_list ap;
		va_start(ap, file);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		
		fwrite("\n\n", sizeof(char), 2, stderr);

#if defined __arm__ || defined __aarch64__



#ifdef __clang__
		//__asm__ volatile("brk #0x1");
		__builtin_debugtrap();
#else
#error "Unsupported compiler"
#endif



#elif defined __i386__ || defined __x86_64__ || defined _WIN32



#if defined __GNUC__
		__asm__ volatile ("int $3");
#elif defined _WIN32
		DebugBreak();
#else
#error "Unsupported compiler"
#endif



#else
#error "Unsupported architecture"
#endif

	}
}





_KSN_END
