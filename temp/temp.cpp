
#include <ksn/math_long_integer.hpp>
#include <ksn/math_constexpr.hpp>

#include <utility>

#include <numeric>


#pragma warning(disable : 4996)

void checkm(uint64_t a, uint64_t b)
{
	ksn::long_uint_stack<2> xa(a), xb(b), r1, r2;

	decltype(xa)::_multiply_default(&r1, &xa, &xb);
	decltype(xa)::__multiply_divide_fft_positive<false>(&r2, &xa, &xb);

	for (size_t i = 0; i < xa.capacity(); ++i)
	{
		if (r1.m_storage.m_data[i] != r2.m_storage.m_data[i])
			throw;
	}
}

void checkd(uint64_t a, uint64_t b)
{
	ksn::long_uint_stack<1> xa(a), xb(b);

	xa /= xb;
	if (xa.m_storage.m_data[0] != a / b)
		throw;
}

auto f()
{
	return std::pair<int, int>{ 1, 2 };
}

//division check
int main()
{
	checkm(1, 1);
	
	checkd(4999999999999999999, 5000000000000000000);

	return 0;
}
