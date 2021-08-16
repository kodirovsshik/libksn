
#include <algorithm>
#include <numeric>
#include <iostream>

#include <ksn/stuff.hpp>
#include <ksn/time.hpp>
#include <ksn/math_constexpr.hpp>

//using bool_t = uint8_t;
using bool_t = bool;

#pragma comment(lib, "libksn_time.lib")

int main()
{

	static constexpr size_t N = 1000000000;

	std::vector<bool_t> is_prime;
	is_prime.resize(N / 2 + 1, 1);
	is_prime[0] = 0; //[0] is for for 0*2+1 = 1

	ksn::stopwatch sw1;
	sw1.start();

	auto check_prime = [&]
	(size_t x)
	{
		return (x & 1) && is_prime[x >> 1];
	};

	auto cut = [&]
	(size_t i)
	{
		if (check_prime(i))
		{
			size_t j = (i * 3 - 1) / 2;
			//i <<= 1;
			for (; j < is_prime.size(); j += i)
			{
				is_prime[j] = false;
			}
		}
	};

	ksn::stopwatch sw;
	sw.start();
	
	constexpr size_t top = ksn::isqrt(N);
	for (size_t i = 3; i <= top; i += 2)
	{
		if (sw.current() > ksn::time::from_sec(1))
		{
			printf("%zu\n", i);
			sw.restart();
		}
		cut(i);
	}

	int c = 1;
	for (size_t i = 1; i < is_prime.size(); i++)
	{
		if (is_prime[i])
			c++;
	}
	printf("%i\n%g s", c, sw1.stop().as_float_sec());

	return 0;
}
