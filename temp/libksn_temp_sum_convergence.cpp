
#include <ksn/math_common.hpp>

int main()
{

	std::vector<double> v{ 1, -4, 6, -4, 1 };

	v = ksn::polynomial_roots(v);

	for (auto x : v) printf("%g ", x);
}
