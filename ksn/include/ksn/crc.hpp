
#ifndef _LIBKSN_CRC_HPP_
#define _LIBKSN_CRC_HPP_



#include <ksn/ksn.hpp>

#include <stdint.h>
#include <stddef.h>



_KSN_BEGIN


uint32_t crc32_init() noexcept;
uint32_t crc32_update(const void* data, size_t size, uint32_t previous) noexcept;
uint32_t crc32(const void* data, size_t size) noexcept;

uint64_t crc64_ecma_init() noexcept;
uint64_t crc64_ecma_update(const void* data, size_t size, uint64_t previous) noexcept;
uint64_t crc64_ecma(const void* data, size_t size) noexcept;


_KSN_END



#endif //!_LIBKSN_CRC_HPP_
