/*

File <ksm/math_formula.hpp>

Contains formulas parsers and evaluators, see comment on the declaration of ksn::_formula_main

*/

#pragma once

#ifndef _KSN_FORMULA_HPP_
#define _KSN_FORMULA_HPP_

#include <ksn/ksn.hpp>
#include <ksn/math_common.hpp>

//#include <ksn/math_complex.hpp>
//#include <ksn/math_long_integer.hpp>

#include <string>
#include <vector>



_KSN_BEGIN

_KSN_DETAIL_BEGIN


struct _formula_base
{
	struct _trie_entry
	{
		uint32_t index;
		uint32_t links[52];
	};

	static const _trie_entry* const _trie_constants;
	static const _trie_entry* const _trie_functions;
	static const uint8_t *const _classify;

	struct _function_descriptor_t
	{
		void* const function_pointer;
		const size_t minimal_arguments_count;
		const size_t maximal_arguments_count;
		const uint8_t takes_parameter_in_name; //0 = does not, 1 = does, 2 = may
	};

	struct parser_helper_t
	{
		size_t current_max = 0;
		size_t max = 0;
	};

	template<typename character_t>
	static character_t _parse_operator_and_extend(const character_t *&p);
};

struct _formula_domain_exception {};


_KSN_DETAIL_END

/*
Math expressions parser and evaluator

Usage:
	double result;
	ksn::_formula_main f;
	int code;

	if ((code = f.parse(expr)) != KSN_FORMULA_PARSE_OK)
	{
		//handle_errors
	}
	else
	{
		result = f.execute({ arg1, arg2, ... argN });
	}


_formula_main::parse(character_t *p, size_t *error_position) result:
Return code == KSN_FORMULA_PARSE_OK: Success
Return code != KSN_FORMULA_PARSE_OK: Failure, parser last position is stored in *error_position if error_position != nullptr
Return code == KSN_FORMULA_PARSE_UNEXCEPTED_ERROR: Unexpected but handled exception
Return code == KSN_FORMULA_PARSE_ALLOCATION_FAILURE: Allocation failure


Expressions guide:

	There are internally 3 types of objects:
		1) A constant
		2) A variable
		3) A function (may or may not be parametrized)
		//4) In fact, there is fourth one called "intermediate", but that are details of implementation

		Constants are like "0", "-1.5", "func(constant [','/';' constants...])"

			Functions of constant value(s) are evaluated during the parsing.


		Variables are like "x", "l0", "A", etc.

			Every time parser finds a new variable, a record about it is made in the object's(ksn::_formula_main) internal table
			and kept until next parse() call. On evaluating evaluator requests all needed variables to be passed at vector.

			Warning: indexes are limited into a range of [0; 2^32 - 2]. Thus, uint32_t(-1) is not a valid index, but uint32_t(-2) is

			Warning2: if you use "W" or "Wn" as a variable and you pass "W(x+1)" a LambertW function will be assumed
				rather than "W*(x+1)". If you wanna you text to be interpreted as multiplication, specify it explicitly
				as shown.

				The same for any other one letter function.
*/
template <class domain_descriptor>
class _formula_main : public detail::_formula_base
{
public:

	using number_t = typename domain_descriptor::number_type;

	size_t m_length;
	size_t m_capacity;
	uint8_t* m_p_data;



	static void* (*g_s_allocator_function)(size_t);
	static void (*g_s_deallocator_function)(void*, size_t);

	static size_t g_s_initial_capacity;



	_formula_main() noexcept;

	template<typename character_t>
	_formula_main(const character_t* expression);

	~_formula_main();



	//Resets the object to just created state
	void reset() noexcept;

	template<typename character_t>
	size_t parse(const character_t* expression, size_t *error_position = nullptr);


	auto execute(const std::vector<number_t> &args) const;
	auto execute(const std::vector<const number_t*> &arg_ptrs) const; 
	auto execute(const number_t *args, size_t count) const;



//private:
	void _extend(size_t);
	void _shrink(size_t);

	_KSN_FORCEINLINE void  _create_storage_if_no_present(size_t = _formula_main::g_s_initial_capacity);
	_KSN_FORCEINLINE void _create_storage_no_check(size_t = _formula_main::g_s_initial_capacity);

	enum class _parsed_data_t : uint8_t
	{
		undefined = 0,
		constant = 1,
		variable = 2,
		intermediate = 3,
		function = 4
	};

#pragma warning(push)
#pragma warning(disable : 26495)
	template <typename character_t>
	struct _parsed_result_t
	{
		using storage_type = size_t;

		typename domain_descriptor::number_type parsed_single_data;
		std::vector<_parsed_result_t> parsed_vector_data;
		size_t parsed_data_length;
		_parsed_data_t parsed_data_type = _parsed_data_t::undefined;

		storage_type storage[3];
	};
#pragma warning(pop)

	template<class character_t>
	size_t _parse_operand(const character_t *p, _parsed_result_t<character_t> &operand, parser_helper_t &ph);

	template<class character_t>
	size_t _function_lookup(const character_t *p, _function_descriptor_t *&descriptor);

	template<class character_t>
	size_t _constant_lookup(const character_t *p, number_t &result);

	template <class character_t>
	size_t _recursive_parser(const character_t *p, parser_helper_t &ph, _parsed_result_t<character_t> &result, character_t previous_operator = character_t(0), _parsed_result_t<character_t> *first = nullptr);

};





struct fpl_domain_descriptor
{
private:
	fpl_domain_descriptor();

public:
	using number_type = long double;

	template <typename character_t>
	static size_t(*const parse_value)(const character_t* p, number_type& result, uint32_t flags);

	static number_type constant_table(size_t index);

	static detail::_formula_base::_function_descriptor_t* function_table(size_t index);
};

template <typename character_t>
size_t(*const  fpl_domain_descriptor::parse_value)(const character_t* p, fpl_domain_descriptor::number_type& result, uint32_t flags) = &parse_fp;// <character_t>;

using formula_ldouble = _formula_main<fpl_domain_descriptor>;





#define KSN_FORMULA_PARSE_OK 0
#define KSN_FORMULA_PARSE_UNEXCEPTED_ERROR 1
#define KSN_FORMULA_PARSE_ALLOCATION_FAILURE 2
#define KSN_FORMULA_PARSE_MISSING_PARAMETER 3
#define KSN_FORMULA_PARSE_UNKNOWN_IDENTIFIER 4
#define KSN_FORMULA_PARSE_MISSING_PARETHESIS 5
#define KSN_FORMULA_PARSE_MISSING_ARGUMENT_SEPARATOR 6
#define KSN_FORMULA_PARSE_DOMAIN_ERROR 7

#define KSN_FORMULA_EXECUTE_OK 0
#define KSN_FORMULA_EXECUTE_DOMAIN_ERROR 1
#define KSN_FORMULA_EXECUTE_ZERO_DIVISION 2
#define KSN_FORMULA_EXECUTE_ALLOCATION_FAILURE 3





_KSN_DETAIL_BEGIN

template<typename character_t>
character_t _formula_base::_parse_operator_and_extend(const character_t*& p)
{
	if (*p == '+')
	{
		++p;
		return '+';
	}
	if (*p == '-')
	{
		++p;
		return '-';
	}
	if (*p == '*')
	{
		++p;
		if (*p == '*')
		{
			++p;
			return '^';
		}
		return '*';
	}
	if (*p == '/')
	{
		++p;
		if (*p == '/')
		{
			++p;
			return '\\';
		}
		return '/';
	}
	if (*p == '%')
	{
		++p;
		return '%';
	}
	if ((p[0] == 'd' || p[0] == 'D') && (p[1] == 'i' || p[1] == 'I') && (p[2] == 'v') || (p[2] == 'V'))
	{
		p += 3;
		return '\\';
	}
	if ((p[0] == 'm' || p[0] == 'M') && (p[1] == 'o' || p[1] == 'O') && (p[2] == 'd' || p[2] == 'D'))
	{
		p += 3;
		return '%';
	}
	return 0;
}

void __cdecl _formula_default_deallocator(void* p, size_t);

_KSN_DETAIL_END



template <class domain_descriptor>
void* (__cdecl* _formula_main<domain_descriptor>::g_s_allocator_function)(size_t) = &malloc;
template <class domain_descriptor>
void(__cdecl* _formula_main<domain_descriptor>::g_s_deallocator_function)(void*, size_t) = &detail::_formula_default_deallocator;



template <class domain_descriptor>
_KSN_FORCEINLINE static void* allocator_wrapper(size_t n)
{
	if (n)
	{
		return _formula_main<domain_descriptor>::g_s_allocator_function(n);
	}
	return nullptr;
}

template <class domain_descriptor>
_KSN_FORCEINLINE static void deallocator_wrapper(void* p, size_t n)
{
	if (p && n)
	{
		_formula_main<domain_descriptor>::g_s_deallocator_function(p, n);
	}
}




template <class domain_descriptor>
_formula_main<domain_descriptor>::_formula_main() noexcept
{
	this->m_p_data = nullptr;
	this->m_length = this->m_capacity = 0;
}

template <class domain_descriptor>
template <typename character_t>
_formula_main<domain_descriptor>::_formula_main(const character_t* str)
{
	this->_create_stotage_no_check();
	this->parse(str);
}

template <class domain_descriptor>
_formula_main<domain_descriptor>::~_formula_main()
{
	deallocator_wrapper<domain_descriptor>(this->m_p_data, this->m_capacity);
}




template <class domain_descriptor>
void _formula_main<domain_descriptor>::reset() noexcept
{
	this->m_length = 0;
}








template <class domain_descriptor>
_KSN_FORCEINLINE void _formula_main<domain_descriptor>::_create_storage_if_no_present(size_t n)
{
	if (this->m_p_data == nullptr)
	{
		this->_create_storage_no_check(n);
	}
}

template <class domain_descriptor>
_KSN_FORCEINLINE void _formula_main<domain_descriptor>::_create_storage_no_check(size_t n)
{
	if (n == 0) n = 1;
	this->m_p_data = (uint8_t*)allocator_wrapper<domain_descriptor>(n);
	this->m_capacity = n;
	this->m_length = 0;
}

















#define skip_spaces(p) { while (*p == ' ') { p++; }; }; ((void)0)

template <class domain_descriptor>
template <class character_t>
size_t _formula_main<domain_descriptor>::_parse_operand(const character_t* p, _parsed_result_t<character_t>& operand, parser_helper_t &ph)
{
	using storage_t = typename _parsed_result_t<character_t>::storage_type;
	//using number_t = typename domain_descriptor::number_type;

	number_t temp_num;
	size_t temp_size;
	const character_t* const p_begin = p;

	//Firstly check if it is something in brackets and we have to call parser to parse the whole brackets
	if (*p == '(')
	{
		++p;
		size_t return_code = this->_recursive_parser(p, ph, operand);
		if (return_code != KSN_FORMULA_PARSE_OK)
		{
			return return_code;
		}

		p += operand.parsed_data_length;
		skip_spaces(p);
		if (*p != ')')
		{
			operand.parsed_data_type = _parsed_data_t::undefined;
			operand.parsed_data_length = p - p_begin + 1;
			return KSN_FORMULA_PARSE_MISSING_PARETHESIS;
		}
		
		return return_code;
	}
	//Now try to find here any constant. Either by lookup tables of known constants or a user-specified one
	else if ((temp_size = domain_descriptor::template parse_value<character_t>(p, temp_num, 15)) || (temp_size = this->_constant_lookup(p, temp_num)))
	{
		operand.parsed_data_length = temp_size;
		operand.parsed_data_type = _parsed_data_t::constant;
		operand.parsed_single_data = std::move(temp_num);
		return KSN_FORMULA_PARSE_OK;
	}
	else if (iswalpha(*p))
	{
		//If we failed to exstract a constant and we have a letter at the beginning of the operand, we may have stumbled upon a variable or a function

		_function_descriptor_t* function;

		//Try to find the function by name using lookyp tables
		if (temp_size = this->_function_lookup(p, function))
		{
			operand.storage[0] = size_t(function->function_pointer);
			operand.storage[1] = 0; //Flags
			operand.storage[2] = 0; //Number of arguments

			//Skip function name
			p += temp_size;

			//If the function can take a parameter in name, such as "log2", "log10", "LambertW0", process it
			if (function->takes_parameter_in_name)
			{
				_parsed_result_t<character_t> primary_parameter;

				//Try to extract the parameter as a constant value
				if (temp_size = parse_fp(p, temp_num))
				{
					p += temp_size;
					primary_parameter.parsed_data_type = _parsed_data_t::constant;
					primary_parameter.parsed_single_data = temp_num;
				}
				else if (*p == '_')
				{
					//"log_(expr1)(expr2)" means "log base (expr1) of (expr2)"
					this->_recursive_parser(p + 1, ph, primary_parameter);
				}

				if (primary_parameter.parsed_data_type == _parsed_data_t::undefined)
				{
					//If no parameter is specified and the function necessary takes the parameter in its name, omit an error
					if (function->takes_parameter_in_name == 1)
					{
						operand.parsed_data_type = _parsed_data_t::undefined;
						operand.parsed_data_length = p - p_begin;
						return KSN_FORMULA_PARSE_MISSING_PARAMETER;
					}
				}

				operand.storage[1] |= 1;
				operand.parsed_vector_data.reserve(function->maximal_arguments_count);
				operand.parsed_vector_data.emplace_back(std::move(primary_parameter));
			}

			const character_t* p_exponent = nullptr;

			skip_spaces(p);
			//If we have something like "f^n(x)"
			if (*p == '^' || (*p == '*' && p[1] == '*'))
			{
				++p;
				skip_spaces(p);
				p_exponent = p; //Process it later simply as a^b, where a and b may be intermediate, constant or whatever
			}


			if (*p != '(')
			{
				//No experssions like "sin x", only "sin(x)"
				operand.parsed_data_type = _parsed_data_t::undefined;
				operand.parsed_data_length = p - p_begin;
				return KSN_FORMULA_PARSE_MISSING_PARETHESIS;
			}
			++p;

			skip_spaces(p);

			//TODO: Process arguments

			size_t i = 0;

			if (function->minimal_arguments_count != 0)
			{
				for (;;)
				{
					//"f(a, b; c, d)"
					_parsed_result_t<character_t> argument;
					size_t code;
					if ((code = this->_recursive_parser(p, ph, argument)) != KSN_FORMULA_PARSE_OK)
					{
						operand.parsed_data_length += p - p_begin;
						operand.parsed_data_type = _parsed_data_t::undefined;
						return code;
					}

					operand.parsed_vector_data.emplace_back(std::move(argument));

					p += argument.parsed_data_length;

					if (++i == function->minimal_arguments_count)
					{
						break;
					}

					skip_spaces(p);
					if (*p != ',' && *p != ';')
					{
						operand.parsed_data_type = _parsed_data_t::undefined;
						operand.parsed_data_length = p - p_begin;
						return KSN_FORMULA_PARSE_MISSING_ARGUMENT_SEPARATOR;
					}

					++p;
					skip_spaces(p);
				}
			}

			


			//TODO: Process power
			//*some more code*

			skip_spaces(p);
			operand.parsed_data_length = p - p_begin;
			if (*p != ')')
			{
				operand.parsed_data_type = _parsed_data_t::undefined;
				return KSN_FORMULA_PARSE_MISSING_PARETHESIS;
			}

			operand.parsed_data_type = _parsed_data_t::function;
			++p;
			operand.parsed_data_length++;

			if (p_exponent != nullptr)
			{
				_parsed_result_t<character_t> result;
				size_t code = this->_recursive_parser<character_t>(p_exponent, ph, result, 0, &operand);
				result.parsed_data_length = (code == KSN_FORMULA_PARSE_OK ? operand.parsed_data_length : result.parsed_data_length + p_exponent - p_begin);
				operand = std::move(result);
				return code;
			}
			else
			{
				return KSN_FORMULA_PARSE_OK;
			}
		}
		else
		{
			//The only hope is that operand is a variable

			//Since variables can consist only of a single letter or a letter with an unsigned number but not of multiple letters, check i there is no two letters
			character_t next_char = p[1];
			if (iswalpha(next_char))
			{
				operand.parsed_data_type = _parsed_data_t::undefined;
				operand.parsed_data_length = p - p_begin;
				return KSN_FORMULA_PARSE_UNKNOWN_IDENTIFIER;
			}
			else
			{
				//We've got "(letter)(lettern't)"

				operand.parsed_data_type = _parsed_data_t::variable;

				uint32_t index = -1;
				character_t letter = *p++;

				if (iswdigit(next_char))
				{
					index = 0;
					do
					{
						index = index * 10 + *p - '0';
					} while (iswdigit(*++p));
				}

				operand.storage[0] = (storage_t)letter;
				operand.storage[1] = (storage_t)index;

				operand.parsed_data_length = p - p_begin;
				return KSN_FORMULA_PARSE_OK;
			}
		}
	}
	

	operand.parsed_data_length = 0;
	operand.parsed_data_type = _parsed_data_t::undefined;
	return KSN_FORMULA_PARSE_OK;
}

template <class domain_descriptor>
template <class character_t>
size_t _formula_main<domain_descriptor>::_constant_lookup(const character_t *p, number_t &result)
{
	const _trie_entry* p_entry = detail::_formula_base::_trie_constants;
	const character_t *const p_begin = p;
	
	while (*p > 0 && *p <= 255)
	{
		uint8_t alpha_type = detail::_formula_base::_classify[*p];
		if (alpha_type == 0) { break; }
		else
		{
			uint32_t offset = 0;
			if (alpha_type == 1)
			{
				offset = p_entry->links[*p - 'a'];
			}
			else if (alpha_type == 2)
			{
				offset = p_entry->links[*p - 'A' + 26];
			}

			p_entry = detail::_formula_base::_trie_constants + offset;

			if (offset == 0)
			{
				break;
			}
		}

		++p;
	}

	if (p_entry->index)
	{
		result = domain_descriptor::constant_table(p_entry->index);
		return p - p_begin;
	}
	else
	{
		return 0;
	}
}

template <class domain_descriptor>
template <class character_t>
size_t _formula_main<domain_descriptor>::_function_lookup(const character_t *p, _function_descriptor_t *&descriptor)
{
	const _trie_entry* p_entry = detail::_formula_base::_trie_functions;
	const character_t* const p_begin = p;

	while (*p > 0 && *p <= 255)
	{
		uint8_t alpha_type = detail::_formula_base::_classify[*p];
		if (alpha_type == 0) { break; }
		else
		{
			uint32_t offset = 0;
			if (alpha_type == 1)
			{
				offset = p_entry->links[*p - 'a'];
			}
			else if (alpha_type == 2)
			{
				offset = p_entry->links[*p - 'A' + 26];
			}

			p_entry = detail::_formula_base::_trie_functions + offset;

			if (offset == 0)
			{
				break;
			}
		}

		++p;
	}

	if (p_entry->index)
	{
		descriptor = domain_descriptor::function_table(p_entry->index);
		return p - p_begin;
	}
	else
	{
		return 0;
	}
}

template <class domain_descriptor>
template <class character_t>
size_t _formula_main<domain_descriptor>::_recursive_parser(
	const character_t *p,
	parser_helper_t &ph,
	_parsed_result_t<character_t> &result,
	character_t previous_operator,
	_parsed_result_t<character_t> *first)
{
	const character_t *p_begin = p;

	_parsed_result_t<character_t> __first;
	_parsed_result_t<character_t> __second, *second = &__second;

	size_t temp;



	skip_spaces(p);

	if (first == nullptr)
	{
		first == &__first;

		size_t code = this->_parse_operand(p, *first, ph);
		if (code != KSN_FORMULA_PARSE_OK)
		{
			result = std::move(*first);
			return code;
		}
		
		p += first->parsed_data_length;
		skip_spaces(p);
	}

	character_t operator1, operator2;
	if ((operator1 = _parse_operator_and_extend(p)) == 0)
	{
		result.parsed_data_length = p - p_begin;
		result.parsed_data_type = _parsed_data_t::undefined;
		return KSN_FORMULA_PARSE_UNKNOWN_IDENTIFIER;
	}

	skip_spaces(p);

	const character_t* p_second_operand = p;
	if ((temp = this->_parse_operand(p, *second, ph)) != KSN_FORMULA_PARSE_OK)
	{
		result.parsed_data_length = p - p_begin + second->parsed_data_length;
		result.parsed_data_type = _parsed_data_t::undefined;
		return temp;
	}

	skip_spaces(p);

	const character_t* p_second_operator = p;

	if ((temp = _parse_operator_and_extend(p)) == KSN_FORMULA_PARSE_OK)
	{
		//We've got an operator after a second argument. Check if it is a higher priority operator
		if (_more_priority_operator(operator2, operator1))
		{
			temp = this->_recursive_parser(p_second_operator, ph, second, operator1, first);
			if (temp != KSN_FORMULA_PARSE_OK)
			{
				result.parsed_data_length = second->parsed_data_length + p_second_operand - p_begin;
				result.parsed_data_type = _parsed_data_t::undefined;
				return temp;
			}
		}
	}

	//Process current operands
	//Create instructions

	/*
	Check if next operator has higher priority then the previous one
		if so, process it first (just by shifting a parse command further on string
		else, ...??
	*/

	throw 0;
}

//"2+2^2^2*2*2"
//"2+16*2*2"


// //"2+2*2^2"

#undef skip_spaces











_KSN_END

#endif //_KSN_FORMULA_HPP_
