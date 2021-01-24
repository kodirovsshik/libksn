#pragma once

#ifndef _KSN_SOCKETS_HPP_
#define _KSN_SOCKETS_HPP_



#include <ksn/ksn.hpp>
#include <stdint.h>
#include <initializer_list>
#include <string>


_KSN_BEGIN



extern void* (*socket_allocator)(size_t size);
extern void (*socket_deallocator)(void* pmemory, size_t size);



enum class socket_status
{
	Ready,
	NotReady,
	Partitial,
	Disconnected,
	Error
};



struct ip_address
{
	union
	{
		uint8_t as_bytes[4];
		uint16_t as_words[2];
		uint32_t as_dword;
	};
	


	ip_address() noexcept;

	ip_address(uint32_t addr) noexcept;
	
	ip_address(const ip_address& other) noexcept;
	ip_address(ip_address&& other) noexcept;

	ip_address(const char* string);
	ip_address(const wchar_t* wstring);
	ip_address(const char16_t* string16);
	ip_address(const char32_t* string32);

	ip_address(std::initializer_list<uint8_t> bytes) noexcept;

	ip_address& operator=(const ip_address& other) noexcept;


	bool operator==(const ip_address& other) const noexcept;
	bool operator!=(const ip_address& other) const noexcept;



	static ip_address address_any; //Any IP
	static ip_address address_localhost; //127.0.0.1
	static ip_address address_boardcast; //255.255.255.255 (UPD broadcast)
	static ip_address address_none; //Invalid IP
};



struct packet
{
	size_t m_size;
	size_t m_capacity;
	size_t m_offset;
	void* m_data;

	bool m_last_read_ok;



	packet() noexcept;
	packet(const packet&) noexcept;
	packet(packet&&) noexcept;

	~packet() noexcept;



	void clear();
	void clear_no_dealloc() noexcept;
	void apply(const void* data, size_t bytes);
	void append(const void* data, size_t bytes);
	size_t unpack(void* buffer, size_t max);

	size_t read_string(char* buffer, size_t buffer_size);
	size_t read_string(wchar_t* buffer, size_t buffer_size);
	size_t read_string(char16_t* buffer, size_t buffer_size);
	size_t read_string(char32_t* buffer, size_t buffer_size);

	packet& operator>>(signed char& data);
	packet& operator>>(unsigned char& data);
	packet& operator>>(signed short& data);
	packet& operator>>(unsigned short& data);
	packet& operator>>(signed int& data);
	packet& operator>>(unsigned int& data);
	packet& operator>>(signed long& data);
	packet& operator>>(unsigned long& data);
	packet& operator>>(signed long long& data);
	packet& operator>>(unsigned long long& data);

	packet& operator>>(bool& data);
	packet& operator>>(wchar_t& data);

	packet& operator>>(std::string& data);
	packet& operator>>(std::wstring& data);
	packet& operator>>(std::u16string& data);
	packet& operator>>(std::u32string& data);

	bool read_pointer(void** pptr);
	size_t get_pending_string_size() const noexcept;


	packet& operator<<(signed char data);
	packet& operator<<(unsigned char data);
	packet& operator<<(signed short data);
	packet& operator<<(unsigned short data);
	packet& operator<<(const signed int data);
	packet& operator<<(unsigned int data);
	packet& operator<<(signed long data);
	packet& operator<<(unsigned long data);
	packet& operator<<(signed long long data);
	packet& operator<<(unsigned long long data);

	packet& operator<<(bool data);
	packet& operator<<(wchar_t data);

	packet& operator<<(const void* pointer);
	packet& operator<<(nullptr_t);

	packet& operator<<(const char* pointer);
	packet& operator<<(const char16_t* pointer);
	packet& operator<<(const char32_t* pointer);
	packet& operator<<(const wchar_t* pointer);

	packet& operator<<(const std::string& data);
	packet& operator<<(const std::wstring& data);
	packet& operator<<(const std::u16string& data);
	packet& operator<<(const std::u32string& data);
	

	operator bool() const noexcept;

	template<class Type>
	Type get();
};



struct socket_base_internal
{
	socket_base_internal() noexcept;
	~socket_base_internal() noexcept;

	uint32_t m_socket;
	bool m_is_blocking;

	void _create(bool);
	void _create(int, bool);

	void _close();

	void _create_addr(uint32_t, uint16_t, void*);
};



struct socket_base : protected socket_base_internal
{
	void set_blocking(bool is_blocking) noexcept;
	bool is_blocking() const noexcept;

	int socket_handle() const noexcept;

	uint16_t get_local_port() const noexcept;
};



struct tcp_socket : public socket_base
{
	friend struct tcp_listener;

	tcp_socket() noexcept;

	tcp_socket(const tcp_socket&) = delete; //just for now, TODO: think about this later
	tcp_socket(tcp_socket&&) = delete;
	


	socket_status connect(ip_address addr, uint16_t port, int64_t microseconds = 0) noexcept;
	void disconnect() noexcept;
	
	socket_status send(const packet& data);
	socket_status send(const void* data, size_t bytes) noexcept;
	socket_status send(const void* data, size_t bytes, size_t& bytes_sent) noexcept;

	socket_status recieve(packet& data);
	socket_status recieve(void* buffer, size_t buffer_size) noexcept;
	socket_status recieve(void* buffer, size_t buffer_size, size_t& recieved) noexcept;



	uint16_t get_remote_port() const noexcept;
	ip_address get_remote_ip() const noexcept;
};



struct tcp_listener : public socket_base
{
	tcp_listener() noexcept;

	tcp_listener(const tcp_listener&) = delete;
	tcp_listener(tcp_listener&&) = delete;



	socket_status accept(tcp_socket& client) noexcept;
	socket_status listen(uint16_t port, ip_address listen_interface = ip_address::address_any, size_t max = -1) noexcept;
	//PAY ATTENTION: Call only if needed only bind without starting listening, otherwise call ksn::tcp_listener::listen instead
	socket_status bind(uint16_t port, ip_address listen_interface = ip_address::address_any) noexcept;

	void close() noexcept;

	void clear_queue(size_t new_size = -1) noexcept;

};



_KSN_END


#include <ksn/sockets.cpp>


#endif //_KSN_SOCKETS_HPP_