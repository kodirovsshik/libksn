
#ifndef _KSN_LONG_INTEGER_HPP_
#define _KSN_LONG_INTEGER_HPP_



#include <ksn/ksn.hpp>

#include <string.h>
#include <math.h>

#include <cmath>

#ifdef _KSN_COMPILER_MSVC
#include <intrin.h>
#else
#include <immintrin.h>
#endif

#include <concepts>
#include <utility>
#include <numeric>
//#include <algorithm>






_KSN_BEGIN



_KSN_DETAIL_BEGIN


using long_integer_limb_t = uint64_t;
using long_integer_signed_limb_t = int64_t;





template<class T>
constexpr void long_integer_fill(T* p, uint8_t pattern, size_t entries)
{
	if (std::is_constant_evaluated())
	{
		uint64_t value = 0x0101010101010101 * pattern;
		while (entries --> 0)
		{
			*p++ = T(value);
		}
	}
	else
	{
		memset(p, pattern, entries * sizeof(T));
	}
}
template<class T>
constexpr void long_integer_copy(T* dst, const T* src, size_t entries)
{
	if (std::is_constant_evaluated())
	{
		if (dst < src)
		{
			while (entries --> 0)
			{
				*dst++ = *src++;
			}
		}
		else
		{
			dst += entries;
			src += entries;
			while (entries --> 0)
			{
				*--dst = *--src;
			}
		}
	}
	else
	{
		memcpy(dst, src, entries * sizeof(T));
	}
}

template<bool is_signed>
constexpr void long_integer_sign_extend(long_integer_limb_t* storage, size_t contained, size_t total)
{
	uint8_t sign = 0;

	if (contained)
	{
		if constexpr (is_signed)
			sign = uint8_t((long_integer_signed_limb_t)storage[contained - 1] >> (sizeof(long_integer_limb_t) * 8 - 1));
		else
			sign = 0;

	}


	long_integer_fill(storage + contained, sign, total - contained);
}





struct long_integer_arithmetic_helper
{
	using limb = long_integer_limb_t;
	using slimb = long_integer_signed_limb_t;

	static constexpr uint8_t ct_addcarry64(uint8_t carry, uint64_t a, uint64_t b, uint64_t* dst)
	{
		uint64_t temp1 = (a & 0xFFFFFFFF) + (b & 0xFFFFFFFF) + carry;
		uint64_t temp2 = (a >> 32) + (b >> 32) + (temp1 >> 32);
		*dst = (temp1 & 0xFFFFFFFF) | (temp2 << 32);
		return (uint8_t)(temp2 >> 32);
	}


	template<class fp_t, std::integral int_t>
	static constexpr long double exp2ict(int_t x) noexcept
	{
		if (std::is_constant_evaluated())
		{
			fp_t result = 1;
			while (x < 0)
			{
				result /= 2;
				++x;
			}
			while (x > 0)
			{
				result *= 2;
				--x;
			}
			return result;
		}
		else
		{
			using std::exp2;
			return exp2((fp_t)x);
		}
	}

	template<class fp_t>
	static constexpr fp_t round_to_zero_ct(fp_t x) noexcept
	{
		if (std::is_constant_evaluated())
		{
			if (x < 0) return -round_to_zero_ct(-x);
			if (x < 1) return 0;

			int base = 1;
			
			do 
			{
				++base;
				x /= 2;
			} while (x >= 1);

			x *= exp2ict<fp_t>(std::numeric_limits<fp_t>::digits);
			base -= std::numeric_limits<fp_t>::digits;

			return (fp_t)(uint64_t)x * exp2ict<fp_t>(base);
		}
		else
		{
			using std::floor;
			return floor(x);
		}
	}
};


struct _long_integer_storage_adapter_heap
{
	using limb = long_integer_limb_t;
	using slimb = long_integer_signed_limb_t;


private:

	limb* m_limbs;
	size_t m_capacity;



	constexpr void deallocate()
	{
		if (this->m_limbs)
			delete[] this->m_limbs;

		this->m_capacity = 0;
		this->m_limbs = nullptr;
	}
	constexpr void allocate(size_t N)
	{
		this->m_capacity = N;
		this->m_limbs = N ? new limb[N] : nullptr;
	}

	template<bool is_signed, bool do_sign_extend>
	constexpr void reverse_base(size_t N)
	{
		if (N <= this->m_capacity)
			return;

		limb* new_limbs = new limb[N];
		long_integer_copy(new_limbs, this->m_limbs, this->m_capacity);

		if constexpr (do_sign_extend)
			detail::long_integer_sign_extend<is_signed>(new_limbs, this->m_capacity, N);

		//std::swap(this->m_limbs, new_limbs); //wtf why doesn't it link
		{
			limb* temp = this->m_limbs;
			this->m_limbs = new_limbs;
			new_limbs = temp;
		}
		this->m_capacity = N;

		delete[] new_limbs;
	}


public:

	constexpr _long_integer_storage_adapter_heap() noexcept
		: m_capacity(0), m_limbs(nullptr)
	{
	}
	constexpr _long_integer_storage_adapter_heap(size_t N)
	{
		this->allocate(N);
	}
	constexpr _long_integer_storage_adapter_heap(size_t N, bool zero_out)
	{
		this->_long_integer_storage_adapter_heap::_long_integer_storage_adapter_heap(N);

		if (zero_out)
			long_integer_fill(this->m_limbs, 0, N);
	}

	constexpr ~_long_integer_storage_adapter_heap() noexcept
	{
		delete[] this->m_limbs;
		this->m_limbs = nullptr;
	}


	constexpr size_t get_capacity() const noexcept
	{
		return this->m_capacity;
	}

	constexpr limb* get_storage() const noexcept
	{
		return this->m_limbs;
	}


	constexpr void reserve_new(size_t N)
	{
		if (N <= this->m_capacity)
			return;

		this->deallocate();
		this->allocate(N);
	}

	template<bool is_signed>
	constexpr void reserve(size_t N)
	{
		return this->reverse_base<is_signed, true>(N);
	}

	constexpr void reserve_no_extend(size_t N)
	{
		return this->reverse_base<false, false>(N);
	}
};

template<size_t N>
struct _long_integer_storage_adapter_stack
{
	using limb = long_integer_limb_t;

	limb m_data[N];



#ifdef _KSN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

	constexpr _long_integer_storage_adapter_stack() noexcept
	{
	}
	constexpr _long_integer_storage_adapter_stack(bool zero_out) noexcept
	{
		if (zero_out)
			long_integer_fill(this->m_data, 0, sizeof(this->m_data) / sizeof(this->m_data[0]));
	}

#ifdef _KSN_COMPILER_MSVC
#pragma warning(pop)
#endif



	constexpr size_t get_capacity() const noexcept
	{
		return N;
	}

	constexpr limb* get_storage() noexcept
	{
		return this->m_data;
	}
	constexpr const limb* get_storage() const noexcept
	{
		return this->m_data;
	}
};



template<class>
struct is_long_integer_storage_adapter_stack
	: std::false_type
{
};

template<size_t N>
struct is_long_integer_storage_adapter_stack<_long_integer_storage_adapter_stack<N>>
	: std::true_type
{
};

template<class test_t>
constexpr static bool is_long_integer_storage_adapter_stack_v = is_long_integer_storage_adapter_stack<test_t>::value;



template<class adapter>
concept storage_adapter =
	std::is_same_v<adapter, _long_integer_storage_adapter_heap> ||
	is_long_integer_storage_adapter_stack_v<adapter>;


_KSN_DETAIL_END





template<detail::storage_adapter adapter_t, bool is_signed>
struct long_integer
{

private:
	
	using my_t = long_integer<adapter_t, is_signed>;



public:

	using adapter_type = adapter_t;
	static constexpr bool is_signed = is_signed;
	
	using limb = detail::long_integer_limb_t;
	using slimb = detail::long_integer_signed_limb_t;



private:

#ifdef _KSN_IS_64

#define _ksnLI_add64v(carry, in1, in2, dst) \
	if (std::is_constant_evaluated())\
		(carry) = _addcarry_u64((carry), in1, in2, dst);\
	else \
		(carry) = detail::long_integer_arithmetic_helper::ct_addcarry64(carry, in1, in2, dst);

#define _ksnLI_add64p(carry, pin1, pin2, dst) \
	if (std::is_constant_evaluated()) \
		(carry) = _addcarry_u64((carry), *(pin1), *(pin2), dst);\
	else\
		(carry) = detail::long_integer_arithmetic_helper::ct_addcarry64((carry), *(pin1), *(pin2), dst);

#else
#endif

	template<class T1, class T2, class To>
	static constexpr void add(To* out, const T1* in1, const T2* in2) noexcept
	{
		size_t s1 = in1->count_occupied_limbs(); //upto 8x slower for smalls. TODO: find a threshold
		size_t s2 = in2->count_occupied_limbs();
		//size_t s1 = in1->capacity();
		//size_t s2 = in2->capacity();
		if (s2 > s1)
		{
			std::swap(s1, s2);
			std::swap(in1, in2);
		}

		size_t so;

		limb sign1 = in1->get_sign(), sign2 = in2->get_sign();

		if constexpr (std::is_same_v<typename To::adapter_type, detail::_long_integer_storage_adapter_heap>)
		{
			so = (s1 > s2 ? s1 : s2);
			if (sign1 == sign2) so += 2;
			if (out->capacity() < so - 1)
				out->reserve(so);
		}
		else
		{
			so = out->capacity();
			if (s1 > so) s1 = so;
			if (s2 > so) s2 = so;
		}

		auto* p1 = in1->m_storage.get_storage();
		auto* p2 = in2->m_storage.get_storage();
		auto* po = out->m_storage.get_storage();

		uint8_t carry = 0;
		size_t i = 0;

		//The loop unroller, i count on you...
		for (; i < s2; ++i)
		{
			_ksnLI_add64p(carry, &p1[i], &p2[i], &po[i]);
		}
		for (; i < s1; ++i)
		{
			_ksnLI_add64v(carry, p1[i], sign2, &po[i]);
		}

		if (i >= so)
			return;

		_ksnLI_add64v(carry, sign1, sign2, &po[i++]);

		limb filler;
		_ksnLI_add64v(carry, sign1, sign2, &filler);
		detail::long_integer_fill(po + i, (uint8_t)filler, so - i);
	}
	
	template<class T1, class T2, class To>
	static constexpr void subtract(To* out, const T1* in1, const T2* in2)
	{
		T2 negated = *in2;
		negated.negate();
		add(out, in1, &negated);
	}



public:

	adapter_t m_storage;





	template<std::integral int_t>
	constexpr void init_from_int(int_t init_value)
	{
		if (this->m_storage.get_capacity() > 0)
		{
			this->m_storage.get_storage()[0] = (limb)init_value;
			detail::long_integer_sign_extend<std::is_signed_v<int_t>>(this->m_storage.get_storage(), 1, this->m_storage.get_capacity());
		}
	}

	constexpr void init_from_fp(long double value)
	{
		using ld = long double;

		bool negate = false;
		if (value < 0)
		{
			negate = true;
			value = -value;
		}

		ld temp;
		ld rem;

		constexpr ld limb_modulo = 4294967296.L;

		limb* p_data = this->m_storage.get_storage();
		limb* const p_data_end = this->m_storage.get_storage() + this->m_storage.get_capacity();

		while (value >= 1 && p_data < p_data_end)
		{
			limb limb_value;

			temp = value / limb_modulo;
			rem = value - detail::long_integer_arithmetic_helper::round_to_zero_ct(temp) * limb_modulo;
			value = temp;
			limb_value = (limb)rem;

			temp = value / limb_modulo;
			rem = value - detail::long_integer_arithmetic_helper::round_to_zero_ct(temp) * limb_modulo;
			value = temp;

			limb_value = limb_value + ((limb)rem << 32);

			*p_data++ = limb_value;
		}

		detail::long_integer_sign_extend<false>(this->m_storage.get_storage(), p_data - this->m_storage.get_storage(), this->m_storage.get_capacity());

		if (negate)
			this->negate();
	}




	constexpr long_integer() {}


	template<std::integral init_t> requires(std::is_same_v<adapter_t, detail::_long_integer_storage_adapter_heap>)
	constexpr long_integer(init_t init_value, size_t init_capacity = 1)
		: m_storage(init_capacity)
	{
		this->init_from_int(init_value);
	}

	template<std::integral init_t> requires(detail::is_long_integer_storage_adapter_stack_v<adapter_t>)
	constexpr long_integer(init_t init_value) noexcept
	{
		this->init_from_int(init_value);
	}


	template<std::floating_point init_t> requires(std::is_same_v<adapter_t, detail::_long_integer_storage_adapter_heap>)
	constexpr long_integer(init_t init_value, size_t init_capacity = 0) noexcept
		: m_storage(0)
	{
		if (init_capacity == 0) //determine automatically
		{
			init_t vabs = fabsf((float)init_value);

			if (vabs >= 1)
				this->m_storage.reserve_new(size_t(log2f((float)vabs) / 64.f) + 1);
			else
			{
				this->m_storage.reserve_new(1);
				this->m_storage.get_storage()[0] = 0;
				return;
			}
		}
		else
			this->m_storage.reserve_new(init_capacity);

		this->init_from_fp(init_value);
	}

	template<std::floating_point init_t> requires(detail::is_long_integer_storage_adapter_stack_v<adapter_t>)
	constexpr long_integer(init_t init_value) noexcept
	{
		this->init_from_fp(init_value);
	}



	_KSN_NODISCARD constexpr my_t operator+() const
	{
		return *this;
	}
	_KSN_NODISCARD constexpr my_t operator-() const
	{
		my_t result(*this);
		result.negate();
		return result;
	}

	_KSN_NODISCARD constexpr my_t operator++(int)
	{
		my_t copy = *this;
		this->increment();
		return copy;
	}
	constexpr my_t& operator++()
	{
		this->increment();
		return *this;
	}



	template<std::integral int_t>
	constexpr operator int_t() const noexcept
	{
		if (this->m_storage.get_capacity() == 0)
			return 0;
		return (int_t)this->m_storage.get_storage()[0];
	}

	template<std::floating_point fp_t>
	constexpr operator fp_t() const noexcept
	{
		if (this->m_storage.get_capacity() == 0)
			return 0;

		if constexpr (is_signed)
			if (this->get_sign() != 0)
				return -(fp_t)-*this;

		size_t limb_index = this->count_occupied_limbs();
		if (limb_index == 0)
			return 0;

		--limb_index;

		return (fp_t)detail::long_integer_arithmetic_helper::exp2ict<fp_t>(limb_index * sizeof(limb) * CHAR_BIT) * this->m_storage.get_storage()[limb_index];

	}



	constexpr void increment()
	{
		limb* p = this->m_storage.get_storage();
		limb* const p_end = p + this->m_storage.get_capacity();

		if (p == p_end)
		{
			if constexpr (std::is_same_v<adapter_t, detail::_long_integer_storage_adapter_heap>)
				this->m_storage.reserve_new(1);
		}
		//else we wouldn't have gotten here as stack vars are never size 0

		limb sign_prev = this->get_sign();

		while (p < p_end)
		{
			if (++ * p)
				break;
			++p;
		}

		if constexpr (std::is_same_v<adapter_t, detail::_long_integer_storage_adapter_heap>)
		{
			bool overflow = false;

			if constexpr (is_signed)
			{
				if (sign_prev == 0 && this->get_sign() != 0)
				{
					size_t prev_cap = this->m_storage.get_capacity();
					this->m_storage.reserve_no_extend(prev_cap * 2);

					detail::long_integer_fill(this->m_storage.get_storage() + prev_cap, 0, prev_cap);
				}
			}
			else
			{
				if (p == p_end)
				{
					size_t prev_cap = this->m_storage.get_capacity();
					this->m_storage.reserve<false>(prev_cap * 2);

					this->m_storage.get_storage()[prev_cap] = 1;
				}
			}
		}
	}

	constexpr void flip_bits() noexcept
	{
		//TODO: bring in SSE and AVX

		limb* p = this->m_storage.get_storage();
		limb* const p_end = p + this->m_storage.get_capacity();

		while (p < p_end)
			*p++ ^= -1;
	}

	constexpr void negate()
	{
		this->flip_bits();
		this->increment();
	}


	constexpr limb get_sign() const noexcept
	{
		if constexpr (!is_signed)
			return 0;
		else
		{
			if (this->m_storage.get_capacity() == 0)
				return 0;
			return (limb)((slimb)this->m_storage.get_storage()[this->m_storage.get_capacity() - 1] >> (sizeof(limb) * CHAR_BIT - 1));
		}
	}

	constexpr size_t capacity() const noexcept
	{
		return this->m_storage.get_capacity();
	}

	constexpr void reserve(size_t new_capacity) noexcept
	{
		this->m_storage.reserve<is_signed>(new_capacity);
	}


	constexpr size_t count_occupied_limbs() const noexcept
	{
		limb sign = this->get_sign();

		if (this->m_storage.get_capacity() == 0)
			return 0;

		if (std::is_constant_evaluated())
		{
			const limb* const p_rend = this->m_storage.get_storage();
			const limb* p = p_rend + this->m_storage.get_capacity() - 1;

			size_t free_limbs = 0;

			while (p >= p_rend)
			{
				if (*p != sign)
					break;
				--p;
			}

			return p - p_rend + 1;
		}
		else
		{
			const limb* const p_rend = this->m_storage.get_storage() - 1;
			const limb* p = p_rend + this->m_storage.get_capacity();

			size_t free_limbs = 0;

			while (p > p_rend)
			{
				if (*p != sign)
					break;
				--p;
			}

			return p - p_rend;
		}
	}

	constexpr size_t count_leading_sign_limbs() const noexcept
	{
		return this->m_storage.get_capacity() - this->count_leading_sign_limbs();
	}


	template<detail::storage_adapter some_adapter_t, bool some_is_signed>
	constexpr my_t& operator+=(const long_integer<some_adapter_t, some_is_signed>& other)
	{
		add(this, this, &other);
		return *this;
	}
	template<detail::storage_adapter some_adapter_t, bool some_is_signed>
	constexpr my_t& operator-=(const long_integer<some_adapter_t, some_is_signed>& other)
	{
		subtract(this, this, &other);
		return *this;
	}
};




using long_int128_t = long_integer<detail::_long_integer_storage_adapter_stack<2>, true>;
using long_int256_t = long_integer<detail::_long_integer_storage_adapter_stack<4>, true>;
using long_int512_t = long_integer<detail::_long_integer_storage_adapter_stack<8>, true>;

using long_uint128_t = long_integer<detail::_long_integer_storage_adapter_stack<2>, false>;
using long_uint256_t = long_integer<detail::_long_integer_storage_adapter_stack<4>, false>;
using long_uint512_t = long_integer<detail::_long_integer_storage_adapter_stack<8>, false>;

template<size_t blocks64bits>
using long_int_stack = long_integer<detail::_long_integer_storage_adapter_stack<blocks64bits>, true>;
template<size_t blocks64bits>
using long_uint_stack = long_integer<detail::_long_integer_storage_adapter_stack<blocks64bits>, false>;

using long_int_heap = long_integer<detail::_long_integer_storage_adapter_heap, true>;
using long_uint_heap = long_integer<detail::_long_integer_storage_adapter_heap, false>;



_KSN_END


#endif //!_KSN_LONG_INTEGER_HPP_
