

#include <ksn/stuff.hpp>

#include <intrin.h>

#include <string.h>

#include <semaphore>
#include <thread>


#ifdef _KSN_COMPILER_MSVC
#define NOMINMAX
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





//Flags:
//Bit 1 << 0 is for capital hex letters
//Bit 1 << 1 is for separating space between bytes
size_t _memory_dump_t::operator()(const void* _p, size_t bytes, size_t bytes_per_line, flag_t flags, FILE* fd)
{
	uint8_t* p = (uint8_t*)_p, * pe = p + bytes;
	size_t byte_in_line = 0;
	char hex_a = flags & 1 ? 'a' : 'A';
	char buffer[4] = { 'x', 'x', ' ', 0};
	size_t len = flags & 2 ? 2 : 3;
	size_t result = 0;

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
			result += fwrite(buffer, sizeof(char), 3, fd);
			buffer[2] = ' ';
		}
		else _KSN_LIKELY
		{
			result += fwrite(buffer, sizeof(char), len, fd);
		}

		++p;
	}

	return result;
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



void memset_parallel(void* void_dst, uint8_t byte, size_t size)
{
	static_assert(std::counting_semaphore<32>::max() == 32);

	if (size < 1024 * 1024 * 32) return (void)memset(void_dst, byte, size);

	uint8_t* dst = (uint8_t*)void_dst;

	//Align by 16 bytes boundary
	while ((uintptr_t)dst & 15 && size)
	{
		*dst++ = byte;
		--size;
	}

	size_t local_size = size / std::thread::hardware_concurrency();
	local_size = std::min(local_size, std::hardware_destructive_interference_size);

	size_t threads_split_count = size / local_size + bool(size % local_size);

	static constexpr size_t N_STATIC_THREADS = 32;
	static thread_local std::thread static_threads[N_STATIC_THREADS];
	std::counting_semaphore<N_STATIC_THREADS> semaphore(std::min(threads_split_count, N_STATIC_THREADS));

	static constexpr auto worker = []
	(void* dst, uint8_t val, size_t size, std::counting_semaphore<N_STATIC_THREADS>& semaphore)
	{
		memset(dst, val, size);
		semaphore.release();
	};

	for (size_t i = 0; size; i = (i + 1) % N_STATIC_THREADS)
	{
		size_t current_local = size > local_size ? local_size : size;

		semaphore.acquire();
		std::thread& current_thread = static_threads[i];
		if (current_thread.joinable()) current_thread.join();
		current_thread = std::thread(worker, dst, byte, current_local, std::ref(semaphore));

		dst += current_local;
		size -= current_local;
	}

	for (auto& thread : static_threads)
		if (thread.joinable()) thread.join();
}



_KSN_END
