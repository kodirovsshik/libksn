#include <ksn/ksn.hpp>
#include <iterator>

_KSN_BEGIN

class string
{
private:
	char32_t* m_pdata;

	size_t m_length;
	size_t m_capacity;

public:
	typedef class iterator iterator;
	typedef class const_iterator const_iterator;
	typedef class reverse_iterator reverse_iterator;
	typedef class const_reverse_iterator const_reverse_iterator;

	class iterator : public std::iterator<std::random_access_iterator_tag, char32_t>
	{
	private:
		string& m_str;
		size_t m_pos;

	public:
		iterator() = delete;
		iterator(const string& str, size_t Position = 0);

		iterator& operator++() noexcept;
		iterator& operator--() noexcept;
		iterator operator++(int) noexcept;
		iterator operator--(int) noexcept;

		bool operator<(const const_iterator& Other) const;
		bool operator<=(const const_iterator& Other) const;
		bool operator>(const const_iterator& Other) const;
		bool operator>=(const const_iterator& Other) const;
		bool operator==(const const_iterator& Other) const;
		bool operator!=(const const_iterator& Other) const;

		iterator& operator=(const iterator& Other) noexcept;
		iterator& operator+=(ptrdiff_t Difference) noexcept;
		iterator& operator-=(ptrdiff_t Difference) noexcept;
		ptrdiff_t operator-=(const iterator& other);

		char32_t& operator[](size_t Index) const;
		char32_t& operator*() const;
	};

	string();
	string(const char* String, size_t From = 0, size_t Length = -1);
	string(const wchar_t* String, size_t From = 0, size_t Length = -1);
	string(const char16_t* String, size_t From = 0, size_t Length = -1);
	string(const char32_t* String, size_t From = 0, size_t Length = -1);

	void assign(const char* String, size_t From = 0, size_t Length = -1);
	void assign(const wchar_t* String, size_t From = 0, size_t Length = -1);
	void assign(const char16_t* String, size_t From = 0, size_t Length = -1);
	void assign(const char32_t* String, size_t From = 0, size_t Length = -1);

	string& operator=(const string& Other);
	string& operator=(const char* String);
	string& operator=(const wchar_t* String);
	string& operator=(const char16_t* String);
	string& operator=(const char32_t* String);

	string& operator+=(char32_t Symbol);
	string& operator+=(const string& String);
	string& operator+=(const char* String);
	string& operator+=(const wchar_t* String);
	string& operator+=(const char16_t* String);
	string& operator+=(const char32_t* String);

	void append(char32_t Symbol);
	void append(const string& String);
	void append(const char* String);
	void append(const wchar_t* String);
	void append(const char16_t* String);
	void append(const char32_t* String);

	void push_back(char32_t Symbol);
	void pop_back();

	size_t length() const noexcept;
	size_t size() const noexcept;
	size_t capacity() const noexcept;

	void reserve(size_t New_Capacity);
	void free_unused();

	char32_t& operator[](size_t Index);
	const char32_t& operator[](size_t Index) const;

	int compare(const string& Other, size_t start1 = 0, size_t start2 = 0, size_t count = -1) const;
	int compare(const char* Other) const;
	int compare(const wchar_t* Other) const;
	int compare(const char16_t* Other) const;
	int compare(const char32_t* Other) const;
};

_KSN_END