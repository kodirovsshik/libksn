
#include <algorithm>
#include <string.h>

#include <chrono>
#include <execution>
#include <semaphore>

#include <ksn/stuff.hpp>

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

int main()
{

	static constexpr size_t N = size_t(1024) * 1024;
	uint8_t* arr = (uint8_t*)malloc(N);
	if (arr == nullptr) return 1;
	memset(arr, 0, N);
	printf("Ready\n");

	constexpr size_t n = 100;

	memset(arr, 0, N);
	float fd3 = 0;
	for (size_t i = 0; i < n; ++i)
	{
		fd3 += ksn::measure_running_time_no_return(memset_parallel, arr, 0, N);
	}
	printf("%10llu ksn::memset_parallel()\n", uint64_t(fd3 / n));

	memset(arr, 0, N);
	float fd2 = 0;
	for (size_t i = 0; i < n; ++i)
	{
		fd2 += ksn::measure_running_time_no_return([&] { std::fill(std::execution::par_unseq, arr, arr + N, 0); });
	}
	printf("%10llu std::fill()\n", uint64_t(fd2 / n));

	memset(arr, 0, N);
	float fd1 = 0;
	for (size_t i = 0; i < n; ++i)
	{
		fd1 += ksn::measure_running_time_no_return(memset, arr, 0, N);
	}
	printf("%10llu memset()\n", uint64_t(fd1 / n));
}