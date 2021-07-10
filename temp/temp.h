#pragma once

#include <ksn/ksn.hpp>

extern "C"  size_t __cdecl frame();

class a_t;
class b_t;

class a_t
{
public:
	consteval a_t();
	consteval a_t(b_t);
};

class b_t
{
public:
	consteval b_t();
	consteval b_t(a_t);
};


KSN_EXTERNAL_LINKAGE_FUNCTION consteval a_t::a_t() {}
KSN_EXTERNAL_LINKAGE_FUNCTION consteval a_t::a_t(b_t)
{
}

KSN_EXTERNAL_LINKAGE_FUNCTION consteval b_t::b_t() {};
KSN_EXTERNAL_LINKAGE_FUNCTION consteval b_t::b_t(a_t) {};