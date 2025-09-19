#pragma once

#include <stdio.h>
#include <stdint.h>
#include <zlib.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct {
  u8 tag;
  u16 len;
} TagLen;

typedef enum {
  TAG_END,
  TAG_BYTE,
  TAG_SHORT,
  TAG_INT,
  TAG_LONG,
  TAG_FLOAT,
  TAG_DOUBLE,
  TAG_BYTE_ARRAY,
  TAG_STRING,
  TAG_LIST,
  TAG_COMPOUND,
  TAG_INT_ARRAY,
  TAG_LONG_ARRAY
} TagID;

typedef struct {
  union {
    FILE *fp;
    gzFile gz;
  };
  bool is_gzip;
} NBTFile;

void error(const char *fmt);

#define assert(CONDITION, TEXT) if (!(CONDITION)) { fprintf(stderr, "\e[31mASSERTION FAILED: %s\e[m\n", TEXT); exit(1); }
