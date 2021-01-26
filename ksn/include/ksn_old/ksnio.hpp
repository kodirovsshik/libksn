#pragma once

#ifndef _KSNIO_HPP_
#define _KSNIO_HPP_

#include <ksn/ksn.hpp>

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#error Nothing is implemented yet

_KSN_BEGIN

int kprintf(const char* format, ...);
int kfprintf(FILE*, const char* format, ...);
int kvprintf(const char* format, va_list);
int kvfprintf(FILE*, const char* format, va_list);

int ksprintf(char* buffer, const char* format, ...);
int kvsprintf(char* buffer, const char* format, va_list);
int ksnprintf(char* buffer, size_t bufferSize, const char* format, ...);
int kvsnprintf(char* buffer, size_t bufferSize, const char* format, va_list);


int kwprintf(const wchar_t* format, ...);
int kfwprintf(FILE*, const wchar_t* format, ...);
int kvwprintf(const wchar_t* format, va_list);
int kvfwprintf(FILE*, const wchar_t* format, va_list);

int kswprintf(wchar_t* buffer, const wchar_t* format, ...);
int kvswprintf(wchar_t* buffer, const wchar_t* format, va_list);
int ksnwprintf(wchar_t* buffer, size_t bufferSize, const wchar_t* format, ...);
int kvsnwprintf(wchar_t* buffer, size_t bufferSize, const wchar_t* format, va_list);


int k16printf(const char16_t* format, ...);
int kf16printf(FILE*, const char16_t* format, ...);
int kv16printf(const char16_t* format, va_list);
int kvf16printf(FILE*, const char16_t* format, va_list);

int ks16printf(char16_t* buffer, const char16_t* format, ...);
int kvs16printf(char16_t* buffer, const char16_t* format, va_list);
int ksn16printf(char16_t* buffer, size_t bufferSize, const char16_t* format, ...);
int kvsn16printf(char16_t* buffer, size_t bufferSize, const char16_t* format, va_list);


int k32printf(const char32_t* format, ...);
int kf32printf(FILE*, const char32_t* format, ...);
int kv32printf(const char32_t* format, va_list);
int kvf32printf(FILE*, const char32_t* format, va_list);

int ks32printf(char32_t* buffer, const char32_t* format, ...);
int kvs32printf(char32_t* buffer, const char32_t* format, va_list);
int ksn32printf(char32_t* buffer, size_t bufferSize, const char32_t* format, ...);
int kvsn32printf(char32_t* buffer, size_t bufferSize, const char32_t* format, va_list);



int kscanf(const char* format, ...);
int kfscanf(FILE*, const char* format, ...);
int kvscanf(const char* format, va_list);
int kvfscanf(FILE*, const char* format, va_list);

int ksscanf(const char* buffer, const char* format, ...);
int kvsscanf(const char* buffer, const char* format, va_list);
int ksnscanf(const char* buffer, size_t bufferSize, const char* format, ...);
int kvsnscanf(const char* buffer, size_t bufferSize, const char* format, va_list);


int kwscanf(const wchar_t* format, ...);
int kfwscanf(FILE*, const wchar_t* format, ...);
int kvwscanf(const wchar_t* format, va_list);
int kvfwscanf(FILE*, const wchar_t* format, va_list);

int kswscanf(const wchar_t* buffer, const wchar_t* format, ...);
int kvswscanf(const wchar_t* buffer, const wchar_t* format, va_list);
int ksnwscanf(const wchar_t* buffer, size_t bufferSize, const wchar_t* format, ...);
int kvsnwscanf(const wchar_t* buffer, size_t bufferSize, const wchar_t* format, va_list);


int k16scanf(const char16_t* format, ...);
int kf16scanf(FILE*, const char16_t* format, ...);
int kv16scanf(const char16_t* format, va_list);
int kvf16scanf(FILE*, const char16_t* format, va_list);

int ks16scanf(const char16_t* buffer, const char16_t* format, ...);
int kvs16scanf(const char16_t* buffer, const char16_t* format, va_list);
int ksn16scanf(const char16_t* buffer, size_t bufferSize, const char16_t* format, ...);
int kvsn16scanf(const char16_t* buffer, size_t bufferSize, const char16_t* format, va_list);


int k32scanf(const char32_t* format, ...);
int kf32scanf(FILE*, const char32_t* format, ...);
int kv32scanf(const char32_t* format, va_list);
int kvf32scanf(FILE*, const char32_t* format, va_list);

int ks32scanf(const char32_t* buffer, const char32_t* format, ...);
int kvs32scanf(const char32_t* buffer, const char32_t* format, va_list);
int ksn32scanf(const char32_t* buffer, size_t bufferSize, const char32_t* format, ...);
int kvsn32scanf(const char32_t* buffer, size_t bufferSize, const char32_t* format, va_list);

_KSN_END



#endif //_KSNIO_HPP_