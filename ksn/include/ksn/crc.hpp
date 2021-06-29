
#ifndef _LIBKSN_CRC_HPP_
#define _LIBKSN_CRC_HPP_



#include <ksn/ksn.hpp>

#include <stdint.h>
#include <stddef.h>



_KSN_BEGIN


uint64_t crc64_ecma_init() noexcept;
uint64_t crc64_ecma(const void* data, size_t size, uint64_t init = crc64_ecma_init()) noexcept;


_KSN_END



#endif //!_LIBKSN_CRC_HPP_
