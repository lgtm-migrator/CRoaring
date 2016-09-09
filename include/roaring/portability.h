/*
 * portability.h
 *
 */

#ifndef ROARING_INCLUDE_PORTABILITY_H_
#define ROARING_INCLUDE_PORTABILITY_H_

#ifndef __APPLE__
#include <malloc.h> // unless we are on a Mac, we include malloc.h
#endif
#include <stdlib.h>

#if __SIZEOF_LONG_LONG__ != 8
#error This code assumes  64-bit long longs (by use of the GCC intrinsics). Your system is not currently supported.
#endif

#if defined(_MSC_VER)
#define __restrict__ __restrict
#endif

// unless DISABLEAVX was defined, if we have AVX2 and BMI2, we enable AVX
#if (!defined(ROARING_USE_AVX)) && (!defined(DISABLEAVX)) && \
    (defined(__AVX2__)) && (defined(__BMI2__))
#define ROARING_USE_AVX
#endif

// if ROARING_USE_AVX was somehow defined and we lack either AVX2 or BMI2, we
// disable it
#if defined(ROARING_USE_AVX) && ((!defined(__AVX2__)) || (!defined(__BMI2__)))
#undef ROARING_USE_AVX
#endif

#if defined(ROARING_USE_AVX) || defined(__x86_64__) || defined(_M_X64)
// we have an x64 processor
#define ROARING_X64
// we include the intrinsic header
#ifdef _MSC_VER
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#else
/* Pretty much anything else. */
#include <x86intrin.h>
#endif
#endif

// if we have AVX, then we use BMI optimizations
#if defined(ROARING_USE_AVX)
#define ROARING_USE_BMI  // we assume that AVX2 and BMI go hand and hand
#define ROARING_USE_AVX2_FOR_DECODING  // optimization
#define ROARING_VECTOR_UNION_ENABLED   // vector unions (optimization)
#endif

#if defined(_MSC_VER)
#define ALIGNED(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED(x) __attribute__((aligned(x)))
#endif
#endif

#ifdef __GNUC__
#define WARN_UNUSED __attribute__((warn_unused_result))
#else
#define WARN_UNUSED
#endif

#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

static inline int hamming(uint64_t x) {
#if defined(ROARING_X64) && defined(__POPCNT__)
    return _mm_popcnt_u64(x);
#else
    // won't work under visual studio, but hopeful we have _mm_popcnt_u64 in
    // many cases
    return __builtin_popcountll(x);
#endif
}

#ifndef UINT64_C
#define UINT64_C(c) (c##ULL)
#endif

#ifndef UINT32_C
#define UINT32_C(c) (c##UL)
#endif

// portable version of  posix_memalign
static inline void *aligned_malloc(size_t alignment, size_t size) {
    void *p;
#ifdef _MSC_VER
    p = _aligned_malloc(size, alignment);
#elif __MINGW32__
    p = __mingw_aligned_malloc(size, alignment);
#else
    if (posix_memalign(&p, alignment, size) != 0) return NULL;
#endif
    return p;
}

#endif /* INCLUDE_PORTABILITY_H_ */