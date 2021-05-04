
#include <algorithm>
#include <string.h>

#include <chrono>
#include <execution>
#include <semaphore>

#include <ksn/try_smol_buffer.hpp>


_KSN_BEGIN

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
	if (local_size < std::hardware_destructive_interference_size) local_size = std::hardware_destructive_interference_size;
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

#include <ksn/math_pplf.hpp>

#include <iostream>

int main()
{
	ksn::pplf pi = ksn::pplf::pi();
	pi /= pi;
	pi -= 1;

	double t = pi;
	t = t;
}
