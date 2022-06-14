
#include <stdio.h>
#include <stdint.h>
#include <math.h>


#pragma warning(disable : 4996)



template<class fp_t, class callable_t>
fp_t limit(callable_t&& f, fp_t&& a, size_t approximation_order, fp_t dx = 0.0001)
{
	const size_t N = approximation_order;

	if (N == 0)
		return 0;

	fp_t coeff = -1;
	size_t num_n = N + 1;
	size_t denom_n = N;

	fp_t x1 = a;
	fp_t x2 = a;

	if (a)
	{
		dx *= a;
		dx = (a + dx) - a;
	}

	fp_t result = 0;

	for (size_t n = 1; n <= N; ++n)
	{
		coeff *= --num_n;
		coeff /= ++denom_n;
		coeff = -coeff;
		x1 += dx;
		x2 -= dx;
		result += coeff * (f(x1) + f(x2));
	}

	return result;
}

double f(double x)
{
	double s = sin(x);
	return 1 / (x * x) - 1 / (s * s);
}

int main()
{



	FILE* fd = fopen("report2.txt", "ab");
	const int width = 22;
	fprintf(fd, "\nf(x) = sqrt(|x|) as x -> 0\n\n| %12s | %*s | %*s | %*s | %*s | %*s |\n\n",
		"N", width, "dx = 0.1", width, "dx = 0.01", width, "dx = 0.001", width, "dx = 0.0001", width, "dx = 0.00001");

	for (uint64_t n = 1; n <= 1000000000; n *= 10)
	{
		fprintf(fd, "| %12llu |", n);
		double dx = 1;
		for (size_t i = 1; i <= 5; ++i)
		{
			dx /= 10;
			fprintf(fd, " %+*.*lf |", width, width - 4, limit<double>(f, 0, n, dx));
			fflush(fd);
		}
		fprintf(fd, "\n");
	}

	fprintf(fd, "\n\n");


	return 0;
}