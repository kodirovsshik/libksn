
#ifndef _KSN_MATH_FORMULA_HPP_
#define _KSN_MATH_FORMULA_HPP_


#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>
#include <ksn/ppvector.hpp>

#include <type_traits>
#include <string>
#include <vector>


_KSN_BEGIN


template<class number_t> requires(
	!std::is_const_v<number_t> &&
	!std::is_volatile_v<number_t> &&
	!std::is_pointer_v<number_t> &&
	!std::is_reference_v<number_t>)
class formula_t
{
public:

	struct parse_result_t
	{
		size_t result = 0;
		size_t pos = 0;

		template<character char_t, class traits_t = std::char_traits<char_t>, class alloc_t = std::allocator<char_t>>
		constexpr operator std::basic_string<char_t, traits_t, alloc_t>() const noexcept;

		enum
		{
			ok = 0,
			bracket_mismatch,
			out_of_memory,
			syntax_error,
			variable_shadowing,
		};
	};

	struct evaluate_result_t
	{
		number_t value;
		size_t error_code = 0;

		constexpr operator number_t() const noexcept(std::is_nothrow_move_constructible_v<number_t>);

		enum
		{
			ok = 0,
			variable_mismatch,
			out_of_memory,
			domain_error,
		}
	}

	class variable_t
	{
	private:
		char32_t letter;
		uint16_t index;

	public:
		template<character char_t>
		constexpr variable_t(const char_t* data) noexcept;

		constexpr bool is_valid() const noexcept;
	};

private:
	std::vector<variable_t> m_expected_vars;
	std::vector<uint8_t> m_sequence;

public:
	template<character char_t>
	constexpr parse_result_t parse(const char_t* str) noexcept;

	template<character char_t, class traits_t = std::char_traits<char_t>, class alloc_t = std::allocator<char_t>>
	constexpr parse_result_t parse(const std::basic_string<char_t, traits_t, alloc_t>& str) noexcept;


	constexpr evaluate_result_t evaluate(const std::vector<std::pair<variable_t, number_t>>& vars) noexcept;

	constexpr evaluate_result_t evaluate(const number_t& single_var) noexcept;

	constexpr evaluate_result_t evaluate() noexcept;

private:
	struct parse_intermediate_result_t
	{
		enum { constant, intermediate, variable};
	};
};



_KSN_END


#endif //!_KSN_MATH_FORMULA_HPP_
