#pragma once

#ifndef _KSN_MATH_FORMULA_HPP_
#define _KSN_MATH_FORMULA_HPP_



#include <ksn/ksn.hpp>
#include <ksn/_formula_instructions.hpp>

#include <functional>
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include <stdexcept>

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>



#ifdef _MSVC_LANG
	#pragma warning(push)
	#pragma warning(disable : 5040 26451)
#endif



_KSN_BEGIN


class formula
{
public:
	void _copy_construct(const formula& other);

	struct variable
	{
		char symbol;
		uint16_t index; //x0, c3, u0170, etc

		variable();
		variable(const variable&);
		template<class CharT>
		variable(const CharT*); //"x", "c0", "c1", etc...

		bool operator<(const variable& other) const noexcept;
		bool operator==(const variable& other) const noexcept;

		operator std::string() const;

		struct hash
		{
			size_t operator()(const variable& object) const noexcept;
		};
	};

private:
	enum class operand_type { constant, variable, intermediate, error = -1 };

	typedef struct
	{
		size_t parsed_length;
		operand_type parsed_type;
		double parsed_result; //a constant, a coded variable or an index of a temporary object
	} parse_result_t;

	template<class CharT>
	static size_t parse_double(const CharT* p, double* pd);

	template<class CharT>
	parse_result_t recursive_parser(const CharT*, size_t&, std::set<variable>&, parse_result_t* = nullptr, char = 0);

	void expand() noexcept(false);
	void expand(size_t) noexcept(false);

	template<class T>
	int do_parse(const T*);

public:

	//Tip: set to 0 to make your code crash
	static size_t g_init_capacity;

	bool static g_perform_bracket_mismatch_check;

	//Do not modify these yourself

	union
	{
		void* m_p_equation;
		uint8_t* m_p8_equation;
		uint16_t* m_p16_equation;
		uint32_t* m_p32_equation;
		uint64_t* m_p64_equation;
	};
	size_t m_length;
	size_t m_capacity;
	std::unordered_map<variable, size_t, variable::hash> m_variables_numbers;


	formula();
	formula(const formula&);
	formula(formula&&);
	~formula();



	formula& operator=(const formula&);



	operator bool() const noexcept;

	double operator()(const std::vector<double>& variables) const;



	/*
	Returns:
		On success: 1
		P.s. any value != 1 is an error

		On failure:
			On unknown exception: 2

			On memory allocation error: 3

			On parse error: -(position of a symbol that caused a parse error) (remember that -0 == 0)
			P.s. -strlen(str) means there is a bracket mismatch and not all brackets were closed
			P.p.s if bracket mismatch found on somewhere in the string (for example, "())" is passed),
				the function returns -(position of an extra bracket) (-2 for "())")
	*/
	template<class CharT>
	int parse(const CharT* expression) noexcept;



	void clear() noexcept;
};



_KSN_END



_KSN_BEGIN

template<class T>
int formula::parse(const T* p) noexcept
{
	int result;

	if (this->m_p_equation == 0)
	{
		this->m_p_equation = malloc(this->g_init_capacity);
		if (!m_p_equation)
		{
			result = 3;
		}
		else
		{
			this->m_capacity = this->g_init_capacity;
		}
	}

	auto parse_retry = [this]
	(const T* p) -> int
	{
		int result;

		try
		{
			result = this->do_parse(p);
		}
		catch (size_t)
		{
			return 3;
		}
		catch (std::bad_alloc)
		{
			return 3;
		}
		catch (...)
		{
			return 2;
		}

		return result;
	};


	//Test for brackets mismatch
	if (this->g_perform_bracket_mismatch_check)
	{
		std::ptrdiff_t counter = 0;
		const T* _p = p;
		while (*_p)
		{
			if (*_p == '(')
				counter++;
			else if (*_p == ')')
				counter--;
			if (counter < 0)
				return p - _p;
			++_p;
		}
		if (counter)
			return p - _p;
	}

	do
	{
		try
		{
			result = this->do_parse(p);
			break;
		}
		catch (size_t x)
		{
			if (this->m_p_equation)
			{
				free(this->m_p_equation);
			}

			this->m_p_equation = malloc(x);
			if (this->m_p_equation)
			{
				this->m_capacity = x;
			}
			else
			{
				result = 3;
				break;
			}
		}
		catch (...)
		{
			result = 3;
			break;
		}
	} while (true);

	if (result != 1)
	{
		this->clear();
		if (this->m_p_equation == 0)
		{
			this->m_capacity = 0;
		}
		else if (this->m_capacity == 0)
		{
			this->m_p_equation = 0;
		}
		//genius
	}

	return result;
}

template<class T>
int formula::do_parse(const T* p_string)
{
	this->clear();

	size_t top_temp_variable = -1;

	size_t top_intermediate = -1;
	std::set<variable> variables;
	parse_result_t parse_result = recursive_parser(p_string, top_intermediate, variables);

	{
		size_t counter = 0;
		for (const auto element : variables)
		{
			this->m_variables_numbers.insert({ element, counter++ });
		}
	}

	if (parse_result.parsed_type == operand_type::error)
		return -(int)parse_result.parsed_length;


	uint8_t* p_buffer = (uint8_t*)(this->m_p_equation) + this->m_length;

	if (parse_result.parsed_type == operand_type::intermediate)
	{
		union
		{
			uint8_t as_byte[2];
			uint16_t as_word;
		} var;
		var.as_word = (uint16_t)(parse_result.parsed_result + 0.5);

		if (var.as_word > INT16_MAX)
		{
			return -1;
		}
		else if (var.as_word < 128)
		{
			this->expand(2);
			//Write two bytes: 0x2A, (low byte of variable number), so it makes "return ith intermediate result" 
			//(should be zeroth, otherwise it means there is a logic error somewhere)
			*(uint16_t*)p_buffer = (uint16_t)(((uint16_t)_KSN_MATH_FORMULA_INSTRUCTION_RET_I) | ((uint16_t)var.as_byte[0] << 8));

			this->m_length += 2;

			return 1;
		}
		else
		{
			this->expand(3);

			*p_buffer++ = _KSN_MATH_FORMULA_INSTRUCTION_RET_I;

			//Since we work with little endian values, 130 is stored in memory like 0x8200 = 0x1000 0010 0000 0000 (bits are shown in BE)
			//And any value > 32767 is like 0b???? ???? 1??? ????
			//So, we reverse the byte order to make sign bit be at the first byte
			*p_buffer++ = var.as_byte[1] | 0b10000000;
			*p_buffer = var.as_byte[0];

			this->m_length += 3;

			return 1;
		}
	}
	else if (parse_result.parsed_type == operand_type::constant)
	{
		this->expand(9);
		this->m_length += 9;

		*p_buffer++ = _KSN_MATH_FORMULA_INSTRUCTION_RET_C;
		*(double*)p_buffer = parse_result.parsed_result;

		return 1;
	}
	else if (parse_result.parsed_type == operand_type::variable)
	{
		this->expand(5);
		this->m_length += 5;

		*p_buffer++ = _KSN_MATH_FORMULA_INSTRUCTION_RET_V;

		*(uint32_t*)p_buffer = *(uint32_t*) & (parse_result.parsed_result);

		return 1;
	}
	else
	{
		this->m_length = 0;
		return -(int)parse_result.parsed_length;
	}
}


template<class CharT>
size_t formula::parse_double(const CharT* p, double* pd)
{
	const auto* p_begin = p;

	if (p == 0 || pd == 0)
		return -1;

	double part_int = 0, part_real = 0;
	size_t exponent = 0;
	bool do_negate = false;

	while (*p == ' ')
		++p;

	if (*p == '-')
	{
		do_negate = true;
		++p;
	}

	if (!isdigit(*p) && *p != '.' && *p != ',')
		return -1;

	//parse int part
	while (isdigit(*p))
		part_int = part_int * 10 + double(*p++ - '0');

	//parse real part (if present)
	if (*p == '.' || *p == ',')
	{
		double divider = 1;
		while (isdigit(*++p))
		{
			part_real = part_real * 10 + double(*p - '0');
			divider *= 10;
		}
		part_real /= divider;
	}

	bool negative_exponent = false;
	//parse exponent (if present)
	if (*p == 'e' || *p == 'E')
	{
		if (*++p == '-')
		{
			negative_exponent = true;
			++p;
		}

		while (isdigit(*p))
		{
			exponent = (exponent << 3) + (exponent << 1) + *p++ - '0';
		}
	}

	part_real += part_int;
	if (exponent)
	{
		part_real = pow(part_real, double(exponent) * (negative_exponent ? -1 : 1));
	}

	if (do_negate)
	{
		part_real = -part_real;
	}

	*pd = part_real;
	return p - p_begin;
};

template<class CharT>
formula::parse_result_t formula::recursive_parser(const CharT* p, size_t& top_intermediate, std::set<variable>& variables, parse_result_t* p_first, char less_priority_token)
{
	//y=x
	//y=x+2
	//y=1+2+3
	//y=((1)(2+0)(3)4(5))	//5!=120
	//y=(1+2)*(3+4)*(5+6) //231
	const CharT* p_begin = p;

	auto parse_operator = []
	(const CharT*& p) -> char
	{
		if (isalpha(*p))
		{
			return '*';
		}

		CharT _operator;
		switch (*p)
		{
		case '(':
			_operator = '*';
			break;

		case '+':
		case '-':
		case '\\':	//div
		case '%':	//mod
		case '^':	//power
			_operator = *p;
			p++;
			break;

		case '*':
			if (p[1] == '*')
			{
				_operator = '^';
				p += 2;
			}
			else
			{
				_operator = '*';
				p++;
			}
			break;
		case '/':
			if (p[1] == '/')
			{
				_operator = '\\'; //div
				p += 2;
			}
			else
			{
				_operator = '/';
				p++;
			}
			break;

		default:
			_operator = 0;
		}
		return (char)_operator;
	};

	auto parse_operand = [&]
	(const CharT*& p, double& operand, operand_type& type) -> void
	{
		size_t parse_c = parse_double(p, &operand);
		if (parse_c != size_t(-1))
		{
			p += parse_c;
			type = operand_type::constant;
		}
		else
		{
			variable var(p);
			if (var.symbol != 0)
			{
				auto variable_to_operand = []
				(const variable& var) -> double
				{
					union
					{
						double as_double;
						uint64_t as_qword;
						uint32_t as_dword;
						uint16_t as_word[2];
						uint8_t as_byte[4];
					};

					as_qword = 0;
					as_byte[0] = var.symbol;
					as_word[1] = var.index;
					return *(double*)&as_double;
				};

				type = operand_type::variable;
				++p;
				if (var.index != uint16_t(-1))
				{
					do
					{
						++p;
					} while (isdigit(*p));
				}

				operand = variable_to_operand(var);
				variables.insert(var);
			}
			else
			{
				if (*p != '(')
				{
					type = operand_type::error;
					return;
				}
				++p;
				parse_result_t parse_result = recursive_parser(p, top_intermediate, variables);
				if (parse_result.parsed_type == operand_type::error)
				{
					type = operand_type::error;;
					return;
				}

				type = parse_result.parsed_type;
				p += parse_result.parsed_length + 1; //')'
				operand = parse_result.parsed_result;
			}
		}
	};

	auto put_convert_instruction_operand = [&]
	(uint8_t*& p_buffer, uint16_t argument) -> void
	{
		if (argument < 128)
		{
			*p_buffer++ = uint8_t(argument);
			this->m_length++;
		}
		else
		{
			union
			{
				uint8_t as_byte[2];
				uint16_t as_word;
			};
			as_word = argument;
			*p_buffer++ = as_byte[1] | INT8_MIN;
			*p_buffer = as_byte[0];
			this->m_length += 2;
		}
	};

	auto put_instruction_operand = [&]
	(uint8_t*& p_buffer, operand_type type, double operand)
	{
		uint8_t* p2 = p_buffer;

		switch (type)
		{
		case operand_type::constant:
			*(double*)p_buffer = operand;
			p_buffer += sizeof(double);
			this->m_length += sizeof(double);
			break;

		case operand_type::variable:
			*(uint32_t*)p_buffer = *(uint32_t*)&operand;
			p_buffer += sizeof(uint32_t);
			this->m_length += sizeof(uint32_t);
			break;

		case operand_type::intermediate:
			put_convert_instruction_operand(p_buffer, uint16_t(operand + 0.5));
			break;

		case operand_type::error:
			throw std::invalid_argument("Operand type \"error\" is passed");
			break;
		}
	};

	auto is_higher_priotiry_operator = []
	(char what, char than) -> bool
	{
		if (what == 0)
			throw std::invalid_argument("Bad operator is passed to the comparator");

		switch (than)
		{
		case 0: //Anything is higher priority that nothing, isn't it?
			return true;

		case '+':
		case '-':
			if (what == '*' || what == '/' || what == '\\' || what == '%')
				return true;

		case '*':
		case '/':
		case '\\':
		case '%':
		case '^':
			if (what == '^')
				return true;

			return false;

		default:
			throw std::invalid_argument("Bad operator is passed to the comparator");
		}
	};

	double operand1, operand2;
	operand_type type1, type2;


	//parse first operand (recursively, if needed)

	if (p_first == nullptr)
		parse_operand(p, operand1, type1);
	else
	{
		operand1 = p_first->parsed_result;
		type1 = p_first->parsed_type;
	}

	if (type1 == operand_type::error)
	{
		parse_result_t result;
		result.parsed_length = p - p_begin;
		result.parsed_type = operand_type::error;
		return result;
	}

	if (*p == ' ')
		++p;



	//Check if we've got only one operand at all, if so, then return

	if (*p == ')' || *p == '\n' || *p == '\0')
	{
		parse_result_t parse_result;
		parse_result.parsed_length = p - p_begin;
		parse_result.parsed_type = type1;
		parse_result.parsed_result = operand1;
		return parse_result;
	}



	//Parse the operator

	char _operator = parse_operator(p);
	if (!_operator)
	{
		parse_result_t result;
		result.parsed_type = operand_type::error;
		result.parsed_length = p - p_begin;
		return result;
	}

	if (*p == ' ')
		++p;

	const CharT* p_second_operand = p;


	//parse second operand (recursievly, if needed)

	parse_operand(p, operand2, type2);
	if (type2 == operand_type::error)
	{
		parse_result_t result;
		result.parsed_length = p - p_begin;
		result.parsed_type = operand_type::error;
		return result;
	}



	//Parse a single operator after this operand. If it has a higher priority, parse it first

	if (*p == ' ')
		++p;

	const CharT* p_second_operand_end = p;

	char _operator2 = parse_operator(p);

	if (_operator2 && is_higher_priotiry_operator(_operator2, _operator))
	{
		parse_result_t parsed_result = this->recursive_parser(p_second_operand, top_intermediate, variables, nullptr, _operator);
		if (parsed_result.parsed_type == operand_type::error)
		{
			parsed_result.parsed_length += p_second_operand - p;
			return parsed_result;
		}

		type2 = parsed_result.parsed_type;
		operand2 = parsed_result.parsed_result;

		p = p_second_operand + parsed_result.parsed_length;
		p_second_operand_end = p;
		_operator2 = parse_operator(p);
		
		//Next operator is guaranteed not to be a higher priority operator
		//Because if it would be, it would have already been processed 
		//by recursive parser called a few lines above
	}

	p = p_second_operand_end;

	//Create an instruction or return a constant

	double result_const;
	bool is_const_result = false;

	size_t destenation; //if constn't

	if (type1 == operand_type::constant && type2 == operand_type::constant)
	{
		is_const_result = true;

		switch (_operator)
		{
		case '+':
			result_const = operand1 + operand2;
			break;

		case '-':
			result_const = operand1 - operand2;
			break;

		case '*':
			result_const = operand1 * operand2;
			break;

		case '/':
			result_const = operand1 / operand2;
			break;

		case '\\':
			double unused;
			result_const = modf(operand1 / operand2, &unused);
			break;

		case '%':
			result_const = fmod(operand1, operand2);
			break;

		case '^':
			result_const = pow(operand1, operand2);
			break;
		}

		type1 = operand_type::constant;
	}
	else
	{
		uint8_t instruction_operand;
		switch (_operator)
		{
		case '+':
			instruction_operand = _KSN_MATH_FORMULA_INSTRUCTION_ADD_CV;
			break;

		case '-':
			instruction_operand = _KSN_MATH_FORMULA_INSTRUCTION_SUB_CV;
			break;

		case '*':
			instruction_operand = _KSN_MATH_FORMULA_INSTRUCTION_MUL_CV;
			break;

		case '/':
			instruction_operand = _KSN_MATH_FORMULA_INSTRUCTION_DIV_CV;
			break;

		case '\\':
			instruction_operand = _KSN_MATH_FORMULA_INSTRUCTION_IDIV_CV;
			break;

		case '%':
			instruction_operand = _KSN_MATH_FORMULA_INSTRUCTION_MOD_CV;
			break;

		case '^':
			instruction_operand = _KSN_MATH_FORMULA_INSTRUCTION_POW_CV;
			break;
		}

		instruction_operand--;
		instruction_operand += 3 * (uint8_t)type1;
		instruction_operand += (uint8_t)type2;

		this->expand(1);

		uint8_t* p_buffer = this->m_p8_equation + this->m_length;

		*p_buffer++ = instruction_operand;
		this->m_length++;

		if (type1 == operand_type::intermediate && type2 == operand_type::intermediate)
		{
			if (abs(int(operand1 + 0.5) - int(operand2 + 0.5)) != 1)
				throw std::logic_error("Internal parser error, intermediate results are broken");

			destenation = size_t((operand1 < operand2 ? operand1 : operand2) + 0.5);
			--top_intermediate;
		}
		else if (type1 == operand_type::intermediate)
		{
			destenation = size_t(operand1 + 0.5);
		}
		else if (type2 == operand_type::intermediate)
		{
			destenation = size_t(operand2 + 0.5);
		}
		else
		{
			destenation = ++top_intermediate;
		}

		put_convert_instruction_operand(p_buffer, uint16_t(destenation));

		put_instruction_operand(p_buffer, type1, operand1);
		put_instruction_operand(p_buffer, type2, operand2);

		type1 = operand_type::intermediate;
	}



	//Continue parsing

	if (*p == ' ')
	{
		++p;
	}

	parse_result_t current;
	if (is_const_result)
	{
		current.parsed_type = operand_type::constant;
		current.parsed_result = result_const;
	}
	else
	{
		current.parsed_type = operand_type::intermediate;
		current.parsed_result = destenation;
	}

	if (_operator2 && is_higher_priotiry_operator(_operator2, less_priority_token))
	{
		parse_result_t parsed_result = this->recursive_parser(p, top_intermediate, variables, &current, less_priority_token);
		parsed_result.parsed_length += p - p_begin;
		return parsed_result;
	}

	current.parsed_length = p - p_begin;
	return current;
};


template<class T>
formula::variable::variable(const T* p)
{
	if (!isalpha(*p))
	{
		this->symbol = 0;
		this->index = 0;
		return;
	}
	this->symbol = (char)*p++;
	this->index = 0;

	if (isdigit(*p))
	{
		this->index = *p++ - '0';
		while (isdigit(*p))
		{
			this->index = (this->index << 3) + (this->index << 1) + (uint32_t(*p++) - '0');
		}

		if (this->index == (uint16_t)-1)
		{
			this->index = this->symbol = 0;
		}
	}
	else
	{
		this->index = -1;
	}
}


_KSN_END



#ifdef _MSVC_LANG
#pragma warning(pop)
#endif



#endif //_KSN_MATH_FORMULA_HPP_