
#include <string.h>

#include <type_traits>
#include <new>
#include <vector>

#include <utility>
#include <concepts>

#include <ksn/math_pplf.hpp>


template<class float_t, std::invocable<size_t> callable>
size_t convergence_speed(callable&& a_n, size_t start = 1, float_t err = 0, float_t *result = nullptr)
{
#define a(n) std::invoke(std::forward<callable>(a_n), n)
	float_t sum = 0;
	if (err < 0) err = -err;
	for (size_t n = start;; ++n)
	{
		float_t current = a(n);
		double t = current;
		float_t temp = sum + current;
		if (abs(temp - sum) <= err && current != 0)
		{
			if (result) *result = sum;
			return n - start + 1;
		}
		sum = temp;
	};
#undef a
}

ksn::pplf f0(size_t n)
{
	return ksn::pplf(1) / ksn::pplf(2 * n + 1) * ksn::pplf((n % 2) ? -1 : 1);
}

ksn::pplf f1(size_t n)
{
	double x0 = 1;
	return f0(n) * (ksn::pplf)exp(-x0 * n);
}

template<class float_t, std::invocable<size_t> callable>
float_t estimate_sum(callable&& a_n, size_t start = 1, float_t err = 1e-6, size_t* pspeed = nullptr)
{
#define a(n) std::invoke(std::forward<callable>(a_n), n)
	float_t k = 1;
	auto f = [=](size_t n) 
	{
		float_t series = (float_t)a_n(n);
		float_t shrinker = pow((float_t)n, k);
		shrinker = exp(-shrinker);
		return series * shrinker; 
	};
	auto dfdx = [=]
	(size_t order)
	{
		return [=]
		(size_t n) 
		{
			float_t multiplier = pow((float_t)n, k);
			return (float_t)a_n(n) * exp(-1 * multiplier) * pow(multiplier, (float_t)order) * float_t((n % 2) ? -1 : 1);
		};
	};

	float_t sum;
	size_t speed = convergence_speed<float_t>(f, start, 0, &sum);

	if (err < 0) err = -err;
	float_t fact = 1;
	for (size_t n = 1; true; n++)
	{
		fact *= n;
		float_t current_term;
		speed += convergence_speed<float_t>(dfdx(n), start, 0, &current_term);
		current_term /= fact;

		float_t new_sum = sum + ((n % 2) ? -current_term : current_term);
		if (abs(new_sum - sum) <= err)
		{
			if (pspeed) *pspeed = speed;
			return new_sum;
		}
		sum = new_sum;
	}
}

double ln(double x)
{
	double y;
	if (x < 1)
		y = (x - 1) / x;
	else if (x < 15)
		y = sqrt(x) - 1;
	else
		y = 3;

	while (1)
	{
		double temp = exp(y);
		double dy = (x - temp) / (x + temp) * 2;

		if (y + dy == y) return y;
		y += dy;
	}
}

int main()
{
	ksn::pplf xa = .75;
	xa = exp(xa);

	ksn::pplf x1, x2;
	size_t s0, s1;
	x2 = estimate_sum<ksn::pplf>(f0, 0, 1e-6, &s1) - ksn::pplf::pi() / 4;
	s0 = convergence_speed(f0, 0, x2, &x1);

	float x = 200 * (int64_t(s0) - s1) / float(s1 + s0);
}
