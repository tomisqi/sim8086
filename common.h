#pragma once
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <assert.h>

#define KB 1024
#define MB 1024*1024
#define BUF_SIZE  1 * MB
#define ENDIAN_SHIFT16(a) (a<<8  | ((a>>8) & 0xff))
#define BYTE_OFFSET(pos_p, start_p) ((U8*)pos_p - (U8*)start_p)

typedef int8_t S8;
typedef uint8_t U8;
typedef int16_t S16;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

struct Buffer
{
  U8* buf_p;
  U64 bufSize;
};

struct String32
{
  char buf[32];
  int  len;
};
