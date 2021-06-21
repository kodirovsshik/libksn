
#ifndef _KSN_MATH_MATRIX_HPP_
#define _KSN_MATH_MATRIX_HPP_


#include <ksn/ksn.hpp>
#include <ksn/math_vec.hpp>



_KSN_BEGIN


template<size_t N, size_t M, class fp_t = float>
class matrix
{
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


	row_t& operator[](size_t i) noexcept
	{
		return data[i];
	}
	row_const_t& operator[](size_t i) const noexcept
	{
		return data[i];
	}

	
	//Matrix-vector multiplication
	template<size_t N1, size_t M1, class fp1_t, class fp2_t>
	friend vec<N1, std::common_type_t<fp1_t, fp2_t>> 
		operator*(const matrix<N1, M1, fp1_t>& lhs, const vec<M1, fp2_t>& rhs)
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

	//Scalar-matrix multiplication
	template<class fpx_t> requires(std::is_convertible_v<fpx_t, fp_t>)
	friend matrix<N, M, std::common_type_t<fp_t, fpx_t>> 
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

	//Matrix-matrix multiplication
	template<size_t A, size_t B, size_t C, class fp1_t, class fp2_t>
	friend matrix<A, C, std::common_type_t<fp1_t, fp2_t>> 
		operator*(const matrix<A, B, fp1_t>& lhs, const matrix<B, C, fp2_t>& rhs)
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

	template<class fpx_t> requires(N == M)
	matrix<N, M, fp_t>&
		operator*=(const matrix<M, N, fpx_t>& rhs)
	{
		return *this = *this * rhs;
	}
};


_KSN_END



#endif //!_KSN_MATH_MATRIX_HPP_
