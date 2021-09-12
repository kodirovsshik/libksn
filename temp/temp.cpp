
#include <iostream>
#include <iomanip>

#include <ksn/math_long_integer.hpp>
#include <ksn/time.hpp>

#pragma comment(lib, "libksn_time")

int main()
{

	constexpr int64_t N = 10000000;
	constexpr size_t limbs = 32;

	ksn::long_int_stack<limbs> x = 0, one = 1;

	for (size_t i = 0; i < limbs / 2; ++i)
	{
		x.m_storage.get_storage()[i] = 1;
	}

	x += one;


	ksn::stopwatch sw;
	sw.start();
	for (size_t i = 0; i < N; ++i)
	{
		x += one;
	}
	auto dt = sw.stop();

	std::cout << (int)*std::launder(&x) << ", " << std::setw(10) << dt.as_nsec() << " ns, " << N / dt.as_float_sec() << "/s\n";

	return 0;
}
