
#ifndef _KSN_MATH_MATRIX_HPP_
#define _KSN_MATH_MATRIX_HPP_


#include <ksn/ksn.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/stuff.hpp>



_KSN_BEGIN


template<size_t N, size_t M, class fp_t = float>
class matrix
{
	using my_t = matrix<N, M, fp_t>;

	template<class...>
	struct is_matrix : std::false_type
	{
	};

	template<size_t N1, size_t M1, class fpx_t>
	struct is_matrix<matrix<N1, M1, fpx_t>> : std::true_type
	{
	};

	template<class... args>
	static constexpr bool is_matrix_v = is_matrix<args...>::value;


	using row_t = fp_t[M];
	using row_const_t = const fp_t[M];


public:

	fp_t data[N][M] = {};


	constexpr row_t& operator[](size_t i) noexcept
	{
		return data[i];
	}
	constexpr row_const_t& operator[](size_t i) const noexcept
	{
		return data[i];
	}


	//Scalar-matrix multiplication
	template<class fpx_t> requires(std::is_convertible_v<fpx_t, fp_t>)
	constexpr friend matrix<N, M, std::common_type_t<fp_t, fpx_t>> 
		operator*(const fpx_t& lhs, const matrix<N, M, fp_t>& rhs)
	{
		using fp_result_t = std::common_type_t<fp_t, fpx_t>;
		matrix<N, M, fp_result_t> result = rhs;

		for (auto& row : result.data)
		{
			for (auto& elem : row)
			{
				elem *= (fp_t)lhs;
			}
		}

		return result;
	}

	//Matrix-scalar multiplication
	template<std::convertible_to<fp_t> fpx_t>
	constexpr friend auto operator*(const matrix<N, M, fp_t>& lhs, const fpx_t& rhs)
	{
		return rhs * lhs;
	}

	template<class ofp_t> requires(N == M)
	constexpr my_t& operator*=(const matrix<M, N, ofp_t>& rhs)
	{
		return *this = *this * rhs;
	}



	template<class = void> requires(N == M)
	constexpr auto inverse() const
	{
		matrix<N, N> copy = *this;
		copy.invert();
		return copy;
	}

	template<class = void> requires(N == M)
	constexpr void invert()
	{
		my_t& left = *this;
		my_t right = matrix<N, N, fp_t>::identity();

		for (size_t i = 0; i < N; ++i)
		{
			if (!left[i][i])
			{
				size_t nonzero = -1;
				for (size_t j = i + 1; j < N; ++j)
				{
					if (left[j][i])
					{
						nonzero = j;
						break;
					}
				}

				if (~nonzero == 0) //nonzero == -1
					throw std::domain_error("Attempt to invert an non-invertable matrix");

				my_t::sub_row(left[i], left[nonzero]);
				my_t::sub_row(right[i], right[nonzero]);
			}

			fp_t factor = 1 / left[i][i];

			for (size_t j = N; j-- > 0;)
			{
				if (j == i) continue;

				fp_t local_factor = factor * left[j][i];

				my_t::sub_row_with_factor(left[j], left[i], local_factor);
				my_t::sub_row_with_factor(right[j], right[i], local_factor);
			}
		}

		for (size_t i = N; i-- > 0;)
		{
			if (left[i][i] == 0)
				throw std::domain_error("Tried to invert an non-invertable matrix");

			fp_t factor = 1 / left[i][i];
			for (size_t j = N; j-- > 0;)
			{
				(*this)[i][j] = right[i][j] * factor;
			}
		}
	}

	template<class = void> requires(N == M)
	constexpr fp_t determinant() const noexcept
	{
		my_t copy = *this;

		for (size_t i = 0; i < N; ++i)
		{
			if (!copy[i][i])
			{
				size_t nonzero = -1;
				for (size_t j = i + 1; j < N; ++j)
				{
					if (copy[j][i])
					{
						nonzero = j;
						break;
					}
				}

				if (~nonzero == 0) //nonzero == -1
					return 0;

				my_t::sub_row(copy[i], copy[nonzero]);
			}

			fp_t factor = 1 / copy[i][i];

			for (size_t j = N; j-- > 0;)
			{
				if (j == i) continue;

				fp_t local_factor = factor * copy[j][i];

				my_t::sub_row_with_factor(copy[j], copy[i], local_factor);
			}
		}

		fp_t result = copy[0][0];
		for (size_t i = 1; i < N; ++i)
		{
			result *= copy[i][i];
		}
		return result;
	}



	constexpr static void sub_row(row_t& from, const row_t& what) noexcept
	{
		for (size_t i = 0; i < M; ++i)
		{
			from[i] -= what[i];
		}
	}

	constexpr static void sub_row_with_factor(row_t& from, const row_t& what, fp_t factor) noexcept
	{
		for (size_t i = 0; i < M; ++i)
		{
			from[i] -= factor * what[i];
		}
	}



	template<class = void> requires(N == M)
	constexpr static auto identity() noexcept
	{
		my_t result;

		for (size_t i = 0; i < N; ++i)
		{
			for (size_t j = 0; j < N; ++j)
			{
				result.data[i][j] = fp_t(i == j);
			}
		}

		return result;
	}
};


//Matrix-matrix multiplication
template<size_t A, size_t B, size_t C, class fp1_t, class fp2_t>
constexpr auto operator*(const matrix<A, B, fp1_t>& lhs, const matrix<B, C, fp2_t>& rhs)
{
	using fp_result_t = std::common_type_t<fp1_t, fp2_t>;
	matrix<A, C, fp_result_t> result;

	for (size_t i = 0; i < A; ++i)
	{
		for (size_t j = 0; j < C; ++j)
		{
			fp_result_t temp = 0;
			for (size_t k = 0; k < B; ++k)
			{
				temp += lhs[i][k] * rhs[k][j];
			}
			result[i][j] = temp;
		}
	}

	return result;
}


//Matrix-vector multiplication
template<size_t N1, size_t M1, class fp1_t, class fp2_t>
constexpr auto operator*(const matrix<N1, M1, fp1_t>& lhs, const vec<M1, fp2_t>& rhs)
{
	using fp_result_t = std::common_type_t<fp1_t, fp2_t>;
	vec<N1, fp_result_t> result;

	for (size_t i = 0; i < N1; ++i)
	{
		fp_result_t temp = 0;
		for (size_t j = 0; j < M1; ++j)
		{
			temp += lhs[i][j] * rhs[j];
		}
		result[i] = temp;
	}

	return result;
}


_KSN_END



#endif //!_KSN_MATH_MATRIX_HPP_
