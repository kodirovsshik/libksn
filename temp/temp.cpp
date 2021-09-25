
#include <ksn/math_long_integer.hpp>

#pragma warning(disable : 4996)

int main()
{

	ksn::long_int_heap a(-1, 4);
	ksn::long_int256_t b(3);
	
	auto c = b * a;

	return 0;
}