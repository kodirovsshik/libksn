
#ifndef _KSN_LONG_INTEGER_HPP_
#define _KSN_LONG_INTEGER_HPP_



#include <ksn/ksn.hpp>
#include <ksn/metapr.hpp>
#include <ksn/math_common.hpp>
#include <ksn/math_complex.hpp>

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
#include <algorithm>






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
	using limb = detail::long_integer_limb_t;
	using slimb = detail::long_integer_signed_limb_t;

	static constexpr uint8_t ct_addcarry64(uint8_t carry, uint64_t a, uint64_t b, uint64_t* dst)
	{
		uint64_t temp1 = (a & 0xFFFFFFFF) + (b & 0xFFFFFFFF) + carry;
		uint64_t temp2 = (a >> 32) + (b >> 32) + (temp1 >> 32);
		*dst = (temp1 & 0xFFFFFFFF) | (temp2 << 32);
		return (uint8_t)(temp2 >> 32);
	}

	static constexpr limb ct_multiply64(limb in1, limb in2, limb* p_high)
	{
		uint64_t l1 = in1 & 0xFFFFFFFF;
		uint64_t l2 = in2 & 0xFFFFFFFF;

		uint64_t h1 = in1 >> 32;
		uint64_t h2 = in2 >> 32;

		uint64_t a, b, c;
		uint8_t carry;

		a = l1 * l2;
		carry = ct_addcarry64(0, l1 * h2, l2 * h1, &b);
		c = h1 * h2 + ((uint64_t)carry << 32);

		uint64_t low, high;
		carry = ct_addcarry64(0, a, b << 32, &low);
		high = c + (b >> 32) + carry;

		*p_high = high;
		return low;
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


template<bool is_signed>
struct _long_integer_storage_adapter_heap
{
	using limb = detail::long_integer_limb_t;
	using slimb = detail::long_integer_signed_limb_t;

	static constexpr bool is_signed = is_signed;


private:

	using my_t = _long_integer_storage_adapter_heap<is_signed>;

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

	template<bool do_sign_extend>
	constexpr void reserve_base(size_t N)
	{
		if (N <= this->m_capacity)
			return;

		limb* new_limbs = new limb[N];
		long_integer_copy(new_limbs, this->m_limbs, this->m_capacity);

		if constexpr (do_sign_extend)
			detail::long_integer_sign_extend<is_signed>(new_limbs, this->m_capacity, N);

		std::swap(this->m_limbs, new_limbs);
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

	constexpr _long_integer_storage_adapter_heap(_long_integer_storage_adapter_heap&& other) noexcept
	{
		this->m_capacity = other.m_capacity;
		this->m_limbs = other.m_limbs;

		other.m_capacity = 0;
		other.m_limbs = nullptr;
	}

	constexpr ~_long_integer_storage_adapter_heap() noexcept
	{
		delete[] this->m_limbs;
		this->m_limbs = nullptr;
	}

	constexpr my_t& operator=(const my_t& other)
	{
		if (this->m_capacity < other.m_capacity)
		{
			this->deallocate();
			this->allocate(other.m_capacity);
		}
		detail::long_integer_copy(this->m_limbs, other.m_limbs, other.m_capacity);
		detail::long_integer_sign_extend<is_signed>(this->m_limbs, other.m_capacity, this->m_capacity);
		return *this;
	}

	constexpr my_t& operator=(my_t&& other) noexcept
	{
		std::swap(this->m_limbs, other.m_limbs);
		std::swap(this->m_capacity, other.m_capacity);
		return *this;
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

	constexpr void reserve(size_t N)
	{
		return this->reserve_base<true>(N);
	}

	constexpr void reserve_no_extend(size_t N)
	{
		return this->reserve_base<false>(N);
	}
};

template<size_t N, bool is_signed>
struct _long_integer_storage_adapter_stack
{
	using limb = long_integer_limb_t;
	constexpr static size_t static_capacity = N;

	static constexpr bool is_signed = is_signed;

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
template<size_t N, bool is_signed>
struct is_long_integer_storage_adapter_stack<_long_integer_storage_adapter_stack<N, is_signed>>
	: std::true_type
{
};

template<class>
struct is_long_integer_storage_adapter_heap
	: std::false_type
{
};
template<bool is_signed>
struct is_long_integer_storage_adapter_heap<_long_integer_storage_adapter_heap<is_signed>>
	: std::true_type
{
};

template<class test_t>
constexpr static bool is_long_integer_storage_adapter_stack_v = is_long_integer_storage_adapter_stack<test_t>::value;
template<class test_t>
constexpr static bool is_long_integer_storage_adapter_heap_v = is_long_integer_storage_adapter_heap<test_t>::value;

template<class adapter>
concept storage_adapter =
	is_long_integer_storage_adapter_heap_v<adapter> ||
	is_long_integer_storage_adapter_stack_v<adapter>;


_KSN_DETAIL_END





template<detail::storage_adapter adapter_t>
struct long_integer
{

private:
	
	using my_t = long_integer<adapter_t>;



public:

	using adapter_type = adapter_t;
	static constexpr bool is_signed = adapter_t::is_signed;
	
	using limb = detail::long_integer_limb_t;
	using slimb = detail::long_integer_signed_limb_t;

//private:

	template<class adapter1_t, class adapter2_t>
	struct long_integer_addsub_result
	{
		constexpr static bool heap_object_present = ksn::is_any_of_v<detail::_long_integer_storage_adapter_heap, adapter1_t, adapter2_t>;

		constexpr static bool result_is_signed = adapter1_t::is_signed || adapter2_t::is_signed;

		using result_adapter_t = std::conditional_t<heap_object_present,
			detail::_long_integer_storage_adapter_heap<result_is_signed>,
			detail::_long_integer_storage_adapter_stack<
				ksn::max_v<adapter1_t::static_capacity, adapter2_t::static_capacity>,
				result_is_signed
			>
		>;

		using type = long_integer<result_adapter_t>;
	};

	template<class adapter1_t, class adapter2_t>
	using long_integer_addsub_result_t = typename long_integer_addsub_result<adapter1_t, adapter2_t>::type;


#ifdef _KSN_IS_64

#define _ksnLI_add64_2c(carry_out, carry_in, in1, in2, dst) \
	if (std::is_constant_evaluated())\
		(carry_out) = detail::long_integer_arithmetic_helper::ct_addcarry64((carry_in), (in1), (in2), (dst));\
	else \
		(carry_out) = _addcarry_u64((carry_in), (in1), (in2), (dst));

#define _ksnLI_add64(carry, in1, in2, dst) _ksnLI_add64_2c(carry, carry, in1, in2, dst)

#define _ksnLI_mul64(in1, in2, plow, phigh) \
	if (std::is_constant_evaluated()) \
		*(plow) = detail::long_integer_arithmetic_helper::ct_multiply64((in1), (in2), (phigh));\
	else \
		*(plow) = _umul128((in1), (in2), (phigh));

#else
#endif

	template<class T1, class T2, class To>
	static constexpr void add(To* out, const T1* in1, const T2* in2) noexcept
	{
		auto get_size_estimation = []
		(const auto* obj)
		{
			if (obj->capacity() && obj->get_top_limb() == obj->get_sign())
				return obj->capacity() - 1;
			else
				return obj->capacity();
		};

		size_t s1, s2;

		limb sign1 = in1->get_sign(), sign2 = in2->get_sign();

		if constexpr (detail::is_long_integer_storage_adapter_heap_v<typename To::adapter_t>)
		{
			s1 = get_size_estimation(in1);
			s2 = get_size_estimation(in2);
		}
		else
		{
			s1 = in1->capacity();
			s2 = in2->capacity();
		}

		//size_t s1 = in1->count_occupied_limbs(); //upto 8x slower for smalls. TODO: find a threshold
		//size_t s2 = in2->count_occupied_limbs();
		//size_t s1 = in1->capacity();
		//size_t s2 = in2->capacity();
		if (s2 > s1)
		{
			std::swap(s1, s2);
			std::swap(in1, in2);
			std::swap(sign1, sign2);
		}

		size_t so;

		if constexpr (detail::is_long_integer_storage_adapter_heap_v<typename To::adapter_t>)
		{
			so = s1;
			if (sign1 == sign2) ++so;
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
			_ksnLI_add64(carry, p1[i], p2[i], &po[i]);
		}
		for (; i < s1; ++i)
		{
			_ksnLI_add64(carry, p1[i], sign2, &po[i]);
		}

		if (i >= so)
			return;

		_ksnLI_add64(carry, sign1, sign2, &po[i++]);

		if ((so -= i) != 0)
		{
			limb filler;
			_ksnLI_add64(carry, sign1, sign2, &filler);
			detail::long_integer_fill(po + i, (uint8_t)filler, so);
		}
	}
	
	template<class T1, class T2, class To>
	static constexpr void subtract(To* out, const T1* in1, const T2* in2)
	{
		T2 negated = *in2;
		negated.negate();
		add(out, in1, &negated);
	}


	template<class data_t>
	static void _fft_vector_shuffle_conditional(std::vector<data_t>& data, const std::vector<size_t> indexes)
	{
		for (size_t i = 0; i < data.size(); ++i)
		{
			if (indexes[i] > i)
				std::swap(data[i], data[indexes[i]]);
		}
	}

	template<bool inverse, class fp_t>
	static void __inplace_FFT_p2(
		std::vector<ksn::complex<fp_t>>& data,
		std::vector<ksn::complex<fp_t>>& roots,
		const std::vector<size_t>& reverse_indexes)
	{
		_fft_vector_shuffle_conditional(data, reverse_indexes);

		if constexpr (inverse)
		{
			for (auto& root : roots)
				root.imag = -root.imag;
		}

		for (size_t current_length = 2, len_log = 1; 
			current_length <= data.size();
			(current_length *= 2), (len_log += 1))
		{
			size_t root_index_pitch = data.size() >> len_log;
			for (size_t i = 0; i < data.size(); i += current_length)
			{
				size_t root_index = 0;
				for (size_t j = 0; j < current_length / 2; ++j)
				{
					ksn::complex<fp_t> u = data[i + j];
					ksn::complex<fp_t> v = data[i + j + current_length / 2] * roots[root_index];
					data[i + j] = u + v;
					data[i + j + current_length / 2] = u - v;
					root_index += root_index_pitch;
				}
			}
		}

		if constexpr (inverse)
		{
			for (auto& entry : data)
				entry.real /= data.size();
		}
	}

	template<bool divide, class T1, class T2, class To>
	static void __multiply_divide_fft_positive(To* out, const T1* in1, const T2* in2)
	{
		using fp_t = double;
		using cfp_t = ksn::complex<fp_t>;
		using int_t = uint32_t;

		static thread_local std::vector<cfp_t> fft_arr1, fft_arr2;
		static thread_local std::vector<cfp_t> roots_arr;
		static thread_local std::vector<size_t> reverse_arr;

		const size_t c1 = in1->count_occupied_limbs();
		const size_t c2 = in2->count_occupied_limbs();

		size_t n = std::max(c1, c2);
		if constexpr (detail::is_long_integer_storage_adapter_stack_v<typename To::adapter_type>)
		{
			if (out->capacity() < n)
				n = out->capacity();
		}

		if (n == 0)
		{
			detail::long_integer_fill(out->m_storage.get_storage(), 0, out->capacity());
			return;
		}

		if (n & (n - 1)) //n is not a power of two
		{
			//Align up to next power of two
			do
			{
				n |= n >> 1;
			} while (n & (n + 1));
			++n;
		}

		if constexpr (!divide)
			n *= 2;

		if constexpr (detail::is_long_integer_storage_adapter_heap_v<typename To::adapter_type>)
			out->reserve(n);

		//Size ratio coefficient
		static constexpr size_t C = sizeof(limb) / sizeof(int_t);
		n *= C;

		size_t log2n = ksn::ilog2(n);

		fft_arr1.resize(n);
		fft_arr2.resize(n);

		const int_t* p1 = (int_t*)in1->m_storage.get_storage();
		const int_t* p2 = (int_t*)in2->m_storage.get_storage();

		for (size_t i = 0; i < c1 * C; ++i)
			fft_arr1[i] = p1[i];
		for (size_t i = 0; i < c2 * C; ++i)
			fft_arr2[i] = p2[i];

		memset(fft_arr1.data() + c1 * C, 0, (n - c1 * C) * sizeof(cfp_t));
		memset(fft_arr2.data() + c2 * C, 0, (n - c2 * C) * sizeof(cfp_t));

		if (reverse_arr.size() != n)
		{
			reverse_arr.resize(n);
			//TODO: try implementing this in ASM with shr and rcl
			for (size_t i = 1; i < n - 1; ++i)
			{
				size_t idx = i;
				size_t rev = 0;
				for (size_t j = 0; j < log2n; ++j)
				{
					rev = (rev << 1) | (idx & 1);
					idx >>= 1;
				}
				reverse_arr[i] = rev;
			}
			reverse_arr[n - 1] = n - 1;
		}

		const size_t roots_size = n / 2;
		if (roots_size)
		{
			if (roots_arr.size() != roots_size && roots_size)
			{
				roots_arr.resize(roots_size);
				roots_arr[0] = 1;
				if (roots_size / 2)
					roots_arr[roots_size / 2] = { 0, -1 };

				//if constexpr (true)
				if (roots_size > 2) //for debug purposes 
					//TODO: make sure this is always true by setting an FFT threshold
				{
					size_t i = 1;
					fp_t angle = fp_t(KSN_PI * 2 / n);
					cfp_t base(std::cos(angle), -std::sin(angle));
					cfp_t current = base;

					while (true)
					{
						roots_arr[i] = current;
						roots_arr[roots_size - i] = { -current.real(), current.imag() };
						if (++i == roots_size / 2)
							break;
						current *= base;
					}
				}
			}
			else
			{
				//Undo conjugation done by FFT
				for (auto& root : roots_arr)
					root.imag = -root.imag;
			}
		}

		__inplace_FFT_p2<false>(fft_arr1, roots_arr, reverse_arr);
		__inplace_FFT_p2<false>(fft_arr2, roots_arr, reverse_arr);
		
		for (size_t i = 0; i < n; ++i)
		{
			if constexpr (divide)
				fft_arr1[i] /= fft_arr2[i];
			else
				fft_arr1[i] *= fft_arr2[i];
		}

		__inplace_FFT_p2<true>(fft_arr1, roots_arr, reverse_arr);


		size_t write_limit = std::min(n, out->capacity() * C);
		int_t* const p_out = (int_t*)out->m_storage.get_storage();

		constexpr fp_t error_adjustment_const = 1 + 2 * std::numeric_limits<fp_t>::epsilon();

		uint64_t accumulator = 0;
		for (size_t i = 0; i < write_limit; ++i)
		{
			double current = fft_arr1[i].real();
			if (current < 0)
			{
				if (current < -1)
					__debugbreak();
				current = 0;
			}
			else
				current *= error_adjustment_const;

			accumulator += (uint64_t)current;
			p_out[i] = int_t(accumulator);
			accumulator >>= (sizeof(int_t)) * 8;
		}

		//TODO: Do we even need this?
		memset(p_out + write_limit, 0, sizeof(int_t) * (out->capacity() * C - write_limit));
	}

	template<bool divide, class T1, class T2, class To>
	static void _multiply_divide_fft(To* out, const T1* in1, const T2* in2)
	{
		if constexpr (!T1::is_signed && !T2::is_signed)
		{
			return __multiply_divide_fft_positive<divide>(out, in1, in2);
		}
		else
		{
			const T1* p1;
			const T2* p2;

			T1 copy1;
			T2 copy2;

			bool sign = false;

			if (in1->get_sign())
			{
				sign = !sign;
				copy1 = *in1;
				copy1.negate();
				p1 = &copy1;
			}
			else
				p1 = in1;

			if (in2->get_sign())
			{
				sign = !sign;
				copy2 = *in2;
				copy2.negate();
				p2 = &copy2;
			}
			else
				p2 = in2;

			__multiply_divide_fft_positive<divide>(out, p1, p2);

			if (sign)
				out->negate();
		}
	}


	template<class T1, class T2, class To>
	static constexpr void _multiply_default(To* _KSN_RESTRICT out, const T1* in1, const T2* in2)
	{
		constexpr bool is_heap = detail::is_long_integer_storage_adapter_heap_v<typename To::adapter_type>;

		size_t so, s1, s2;
		if constexpr (is_heap)
		{
			s1 = in1->count_occupied_limbs();
			s2 = in2->count_occupied_limbs();
			so = s1 + s2;
			if constexpr (is_signed)
				++so;
			out->reserve(so);

			if (s1 > s2)
				std::swap(in1, in2);
		}
		else
		{
			s1 = in1->capacity();
			s2 = in2->capacity();

			if (s1 > s2)
				return _multiply_default(out, in2, in1);

			so = out->capacity();

			if (s1 > so) s1 = so;
			if (s2 > so) s2 = so;
		}

		const limb* const p1 = in1->m_storage.get_storage();
		const limb* const p2 = in2->m_storage.get_storage();
		limb* const po = out->m_storage.get_storage();
		detail::long_integer_fill(po, 0, so);

		for (size_t i = 0; i < so; ++i)
		{
			const size_t top = is_heap ? s2 : (so - i);
			
			limb carry0 = 0;
			uint8_t carry1 = 0;

			size_t j = 0;
			for (; j < top; ++j)
			{
				limb l = i < s1 ? p1[i] : in1->get_sign();
				limb h = j < s2 ? p2[j] : in2->get_sign();
				_ksnLI_mul64(l, h, &l, &h);
				_ksnLI_add64_2c(carry1, 0, l, carry0, &l);
				h += carry1;

				_ksnLI_add64_2c(carry1, 0, l, po[i + j], &po[i + j]);
				_KSN_DEBUG_EXPR(if (h > 0xFFFFFFFFFFFFFFFF - carry1) __debugbreak());
				
				h += carry1;

				carry0 = h;
			}
			
			while (carry0 && j < so - i)
			{
				_ksnLI_add64_2c(carry0, 0, po[i + j], carry0, &po[i + j]);
				++j;
			}
		}
	}

	template<class T1, class T2, class To>
	static constexpr void _divide_default(To* _KSN_RESTRICT out, const T1* in1, const T2* in2)
	{
		//TODO: constexpr division algorithm
	}

	template<bool divide, class T1, class T2, class To>
	static bool use_fft(To* out, const T1* in1, const T2* in2)
	{
		static constexpr size_t fft_multiply_threshold = 0;
		static constexpr size_t fft_divide_threshold = 0;

		//TODO: thresholds
		//static constexpr size_t fft_multiply_threshold = ...;
		//static constexpr size_t fft_divide_threshold = ...;
		size_t n;
		if constexpr (detail::is_long_integer_storage_adapter_stack_v<typename To::adapter_type>)
			n = out->capacity();
		else
			n = std::max(in1->capacity(), in2->capacity());

		if constexpr (divide)
			return n >= fft_divide_threshold;
		else
			return n >= fft_multiply_threshold;
	}

	template<bool first_two_same, bool divide, class T1, class T2, class To>
	static constexpr void multiply_or_divide(To* out, const T1* in1, const T2* in2)
	{
		if (!std::is_constant_evaluated() && use_fft<divide>(out, in1, in2))
		{
			_multiply_divide_fft<divide>(out, in1, in2);
		}
		else if constexpr (first_two_same)
		{
			To aux;
			if constexpr (divide)
				_divide_default(&aux, in1, in2);
			else
				_multiply_default(&aux, in1, in2);
			*out = std::move(aux);
		}
		else
			if constexpr (divide)
				_divide_default(out, in1, in2);
			else
				_multiply_default(out, in1, in2);
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

	template<std::integral int_t>
	constexpr void init_from_init_list(const std::initializer_list<int_t>& values)
	{
		auto* my_storage = this->m_storage.get_storage();

		if (!std::is_constant_evaluated() && ksn::is_any_of_v<int_t, limb, slimb>)
		{
			size_t to_be_copied = std::min(this->capacity(), values.size());
			memcpy(my_storage, values.begin(), to_be_copied * sizeof(int_t));
		}
		else
			std::copy(values.begin(), values.end(), my_storage);

		detail::long_integer_sign_extend<is_signed>(my_storage, values.size(), this->capacity());
	}
	
	template<detail::storage_adapter some_adapter_t> requires(detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
	constexpr void init_from_other(const long_integer<some_adapter_t>& other)
	{
		static constexpr bool some_is_signed = some_adapter_t::is_signed;

		size_t reserve_additional = 0;
		if constexpr (is_signed && !some_is_signed)
			if (other.get_sign() == -1 && other.get_top_limb() != limb(-1))
				reserve_additional = 1;

		this->reserve(other.capacity() + reserve_additional);
		detail::long_integer_copy(this->m_storage.get_storage(), other.m_storage.get_storage(), other.capacity());

		if (reserve_additional)
			this->m_storage.get_storage()[other.capacity()] = 0;
	}

	template<detail::storage_adapter some_adapter_t> requires(detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
	constexpr void init_from_other(long_integer<some_adapter_t>&& other)
	{
		static constexpr bool some_is_signed = some_adapter_t::is_signed;

		bool good = false;

		if constexpr (std::is_same_v<some_adapter_t, detail::_long_integer_storage_adapter_heap>)
		{
			if constexpr (is_signed != some_is_signed)
			{
				if (other.get_sign() == 0)
					good = true;
			}
			else
				good = true;

			if (good)
			{
				this->m_storage = std::move(other.m_storage);
				return;
			}
		}

		if (!good)
			this->init_from_other((const long_integer<some_adapter_t>&)other);
	}

	template<detail::storage_adapter some_adapter_t> requires(detail::is_long_integer_storage_adapter_stack_v<adapter_t>)
	constexpr void init_from_other(const long_integer<some_adapter_t>& other)
	{
		size_t to_copy = std::min(this->capacity(), other.capacity());
		detail::long_integer_copy(this->m_storage.get_storage(), other.m_storage.get_storage(), to_copy);
		detail::long_integer_sign_extend(this->m_storage.get_storage(), to_copy, this->capacity());
	}



	_KSN_NODISCARD constexpr long_integer() {}


	template<detail::storage_adapter some_adapter_t>
	_KSN_NODISCARD constexpr long_integer(const long_integer<some_adapter_t>& other)
	{
		this->init_from_other(other);
	}

	template<detail::storage_adapter some_adapter_t>
	_KSN_NODISCARD constexpr long_integer(long_integer<some_adapter_t>&& other)
	{
		this->init_from_other(other);
	}

	template<>
	_KSN_NODISCARD constexpr long_integer(my_t&& other) noexcept
	{
		this->init_from_other(other);
	}


	template<std::integral init_t> requires(detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
	_KSN_NODISCARD constexpr long_integer(init_t init_value, size_t init_capacity = 1)
		: m_storage(init_capacity)
	{
		this->init_from_int(init_value);
	}

	template<std::integral init_t> requires(detail::is_long_integer_storage_adapter_stack_v<adapter_t>)
	_KSN_NODISCARD constexpr long_integer(init_t init_value) noexcept
	{
		this->init_from_int(init_value);
	}


	template<std::floating_point init_t> requires(detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
	_KSN_NODISCARD constexpr long_integer(init_t init_value, size_t init_capacity = 0)
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
	_KSN_NODISCARD constexpr long_integer(init_t init_value) noexcept
	{
		this->init_from_fp(init_value);
	}


	template<std::integral int_t> requires (detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
	_KSN_NODISCARD constexpr long_integer(const std::initializer_list<int_t>& values)
		: m_storage(values.size())
	{
		this->init_from_init_list(values);
	}
	
	template<std::integral int_t> requires (detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
	_KSN_NODISCARD constexpr long_integer(const std::initializer_list<int_t>& values, size_t init_size)
		: m_storage(init_size)
	{
		this->init_from_init_list(values);
	}
	
	template<std::integral int_t> requires (detail::is_long_integer_storage_adapter_stack_v<adapter_t>)
	_KSN_NODISCARD constexpr long_integer(const std::initializer_list<int_t>& values) noexcept
	{
		this->init_from_init_list(values);
	}



	template<detail::storage_adapter some_adapter_t>
	my_t& operator=(const long_integer<some_adapter_t>& other)
	{
		this->init_from_other(other);
		return *this;
	}

	template<detail::storage_adapter some_adapter_t>
	my_t& operator=(long_integer<some_adapter_t>&& other)
	{
		this->init_from_other(other);
		return *this;
	}

	template<>
	my_t& operator=(const my_t& other)
	{
		this->m_storage = other.m_storage;
		return *this;
	}

	template<>
	my_t& operator=(my_t&& other) noexcept
	{
		this->init_from_other(std::forward<my_t>(other));
		return *this;
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

	template<class xadapter_t>
	_KSN_NODISCARD constexpr auto operator+
	(const long_integer<xadapter_t>& other)
	{
		long_integer_addsub_result_t<adapter_t, xadapter_t> result;
		add(&result, this, &other);
		return result;
	}


	template<std::integral int_t>
	_KSN_NODISCARD constexpr operator int_t() const noexcept
	{
		if (this->m_storage.get_capacity() == 0)
			return 0;
		return (int_t)this->m_storage.get_storage()[0];
	}

	template<std::floating_point fp_t>
	_KSN_NODISCARD constexpr operator fp_t() const noexcept
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
			if constexpr (detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
				this->m_storage.reserve_new(1);
		}

		limb sign_prev = this->get_sign();

		while (p < p_end)
		{
			if (++ * p)
				break;
			++p;
		}

		if constexpr (detail::is_long_integer_storage_adapter_heap_v<adapter_t>)
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


	_KSN_NODISCARD constexpr limb get_top_limb() const noexcept
	{
		if (this->m_storage.get_capacity() == 0)
			return 0;
		return this->m_storage.get_storage()[this->m_storage.get_capacity() - 1];
	}
	
	_KSN_NODISCARD constexpr limb get_sign() const noexcept
	{
		if constexpr (!is_signed)
			return 0;
		else
		{
			return (limb)((slimb)this->get_top_limb() >> (sizeof(limb) * CHAR_BIT - 1));
		}
	}


	_KSN_NODISCARD constexpr size_t capacity() const noexcept
	{
		return this->m_storage.get_capacity();
	}

	constexpr void reserve(size_t new_capacity) noexcept
	{
		this->m_storage.reserve(new_capacity);
	}


	_KSN_NODISCARD constexpr size_t count_occupied_limbs() const noexcept
	{
		const limb sign = this->get_sign();

		if (this->m_storage.get_capacity() == 0)
			return 0;

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

	_KSN_NODISCARD constexpr size_t count_leading_sign_limbs() const noexcept
	{
		return this->m_storage.get_capacity() - this->count_leading_sign_limbs();
	}



	template<detail::storage_adapter some_adapter_t>
	constexpr my_t& operator+=(const long_integer<some_adapter_t>& other)
	{
		add(this, this, &other);
		return *this;
	}
	
	template<detail::storage_adapter some_adapter_t>
	constexpr my_t& operator-=(const long_integer<some_adapter_t>& other)
	{
		subtract(this, this, &other);
		return *this;
	}

	template<detail::storage_adapter some_adapter_t>
	constexpr my_t& operator*=(const long_integer<some_adapter_t>& other)
	{
		multiply_or_divide<true, false>(this, this, &other);
		return *this;
	}

	template<detail::storage_adapter some_adapter_t>
	constexpr my_t& operator/=(const long_integer<some_adapter_t>& other)
	{
		multiply_or_divide<true, true>(this, this, &other);
		return *this;
	}



#undef  _ksnLI_add64_2c

#undef _ksnLI_add64

#undef _ksnLI_mul64
};




using long_int128_t = long_integer<detail::_long_integer_storage_adapter_stack<2, true>>;
using long_int256_t = long_integer<detail::_long_integer_storage_adapter_stack<4, true>>;
using long_int512_t = long_integer<detail::_long_integer_storage_adapter_stack<8, true>>;

using long_uint128_t = long_integer<detail::_long_integer_storage_adapter_stack<2, false>>;
using long_uint256_t = long_integer<detail::_long_integer_storage_adapter_stack<4, false>>;
using long_uint512_t = long_integer<detail::_long_integer_storage_adapter_stack<8, false>>;

template<size_t blocks64bits>
using long_int_stack = long_integer<detail::_long_integer_storage_adapter_stack<blocks64bits, true>>;
template<size_t blocks64bits>
using long_uint_stack = long_integer<detail::_long_integer_storage_adapter_stack<blocks64bits, false>>;

using long_int_heap = long_integer<detail::_long_integer_storage_adapter_heap<true>>;
using long_uint_heap = long_integer<detail::_long_integer_storage_adapter_heap<false>>;



_KSN_END


#endif //!_KSN_LONG_INTEGER_HPP_
