/*
 * Project : SIMD_Utils
 * Version : 0.2.2
 * Author  : JishinMaster
 * Licence : BSD-2
 */

#pragma once

#include <stdint.h>
#ifndef ARM
#include <immintrin.h>
#else
#include "sse2neon_wrapper.h"
#endif

static inline void add128s(int32_t *src1, int32_t *src2, int32_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT32;
    stop_len *= SSE_LEN_INT32;

    if (areAligned3((uintptr_t) (src1), (uintptr_t) (src2), (uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_store_si128((__m128i *) (dst + i), _mm_add_epi32(_mm_load_si128((__m128i *) (src1 + i)),
                                                                 _mm_load_si128((__m128i *) (src2 + i))));
        }
    } else {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_storeu_si128((__m128i *) (dst + i), _mm_add_epi32(_mm_loadu_si128((__m128i *) (src1 + i)),
                                                                  _mm_loadu_si128((__m128i *) (src2 + i))));
        }
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = src1[i] + src2[i];
    }
}


// result is wrong, the instruction casts to 64bit
#if 0
static inline void mul128s(int32_t *src1, int32_t *src2, int32_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT32;
    stop_len *= SSE_LEN_INT32;

    if (areAligned3((uintptr_t) (src1), (uintptr_t) (src2), (uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_store_si128((__m128i *) dst + i, _mm_mul_epi32(_mm_load_si128((__m128i *) (src1 + i)), _mm_load_si128((__m128i *) (src2 + i))));
        }
    } else {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_storeu_si128((__m128i *) dst + i, _mm_mul_epi32(_mm_loadu_si128((__m128i *) (src1 + i)), _mm_loadu_si128((__m128i *) (src2 + i))));
        }
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = src1[i] * src2[i];
    }
}
#endif

static inline void sub128s(int32_t *src1, int32_t *src2, int32_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT32;
    stop_len *= SSE_LEN_INT32;

    if (areAligned3((uintptr_t) (src1), (uintptr_t) (src2), (uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_store_si128((__m128i *) (dst + i), _mm_sub_epi32(_mm_load_si128((__m128i *) (src1 + i)),
                                                                 _mm_load_si128((__m128i *) (src2 + i))));
        }
    } else {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_storeu_si128((__m128i *) (dst + i), _mm_sub_epi32(_mm_loadu_si128((__m128i *) (src1 + i)),
                                                                  _mm_loadu_si128((__m128i *) (src2 + i))));
        }
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = src1[i] - src2[i];
    }
}

static inline void addc128s(int32_t *src, int32_t value, int32_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT32;
    stop_len *= SSE_LEN_INT32;

    const v4si tmp = _mm_set1_epi32(value);

    if (areAligned2((uintptr_t) (src), (uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_store_si128((__m128i *) (dst + i), _mm_add_epi32(tmp, _mm_load_si128((__m128i *) (src + i))));
        }
    } else {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            _mm_storeu_si128((__m128i *) (dst + i), _mm_add_epi32(tmp, _mm_loadu_si128((__m128i *) (src + i))));
        }
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = src[i] + value;
    }
}

static inline void vectorSlope128s(int *dst, int len, int offset, int slope)
{
    v4si coef = _mm_set_epi32(3 * slope, 2 * slope, slope, 0);
    v4si slope8_vec = _mm_set1_epi32(8 * slope);
    v4si curVal = _mm_add_epi32(_mm_set1_epi32(offset), coef);
    v4si curVal2 = _mm_add_epi32(_mm_set1_epi32(offset), coef);
    curVal2 = _mm_add_epi32(curVal2, _mm_set1_epi32(4 * slope));

    int stop_len = len / (2 * SSE_LEN_INT32);
    stop_len *= (2 * SSE_LEN_INT32);

    if (isAligned((uintptr_t) (dst), SSE_LEN_BYTES)) {
        _mm_store_si128((__m128i *) dst, curVal);
        _mm_store_si128((__m128i *) (dst + SSE_LEN_INT32), curVal2);
    } else {
        _mm_storeu_si128((__m128i *) dst, curVal);
        _mm_storeu_si128((__m128i *) (dst + SSE_LEN_INT32), curVal2);
    }

    if (isAligned((uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 2 * SSE_LEN_INT32; i < stop_len; i += 2 * SSE_LEN_INT32) {
            curVal = _mm_add_epi32(curVal, slope8_vec);
            _mm_store_si128((__m128i *) (dst + i), curVal);
            curVal2 = _mm_add_epi32(curVal2, slope8_vec);
            _mm_store_si128((__m128i *) (dst + i + SSE_LEN_INT32), curVal2);
        }
    } else {
        for (int i = 2 * SSE_LEN_INT32; i < stop_len; i += 2 * SSE_LEN_INT32) {
            curVal = _mm_add_epi32(curVal, slope8_vec);
            _mm_storeu_si128((__m128i *) (dst + i), curVal);
            curVal2 = _mm_add_epi32(curVal2, slope8_vec);
            _mm_storeu_si128((__m128i *) (dst + i + SSE_LEN_INT32), curVal2);
        }
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = offset + slope * i;
    }
}

static inline void sum128s(int32_t *src, int32_t *dst, int len)
{
    int stop_len = len / (2 * SSE_LEN_INT32);
    stop_len *= (2 * SSE_LEN_INT32);

    __attribute__((aligned(SSE_LEN_BYTES))) int32_t accumulate[SSE_LEN_INT32] = {0, 0, 0, 0};
    int32_t tmp_acc = 0;
    v4si vec_acc1 = _mm_setzero_si128();  // initialize the vector accumulator
    v4si vec_acc2 = _mm_setzero_si128();  // initialize the vector accumulator

    if (areAligned2((uintptr_t) (src), (uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 0; i < stop_len; i += 2 * SSE_LEN_INT32) {
            v4si vec_tmp1 = _mm_load_si128((__m128i *) (src + i));
            vec_acc1 = _mm_add_epi32(vec_acc1, vec_tmp1);
            v4si vec_tmp2 = _mm_load_si128((__m128i *) (src + i + SSE_LEN_INT32));
            vec_acc2 = _mm_add_epi32(vec_acc2, vec_tmp2);
        }
    } else {
        for (int i = 0; i < stop_len; i += 2 * SSE_LEN_INT32) {
            v4si vec_tmp1 = _mm_loadu_si128((__m128i *) (src + i));
            vec_acc1 = _mm_add_epi32(vec_acc1, vec_tmp1);
            v4si vec_tmp2 = _mm_load_si128((__m128i *) (src + i + SSE_LEN_INT32));
            vec_acc2 = _mm_add_epi32(vec_acc2, vec_tmp2);
        }
    }
    vec_acc1 = _mm_add_epi32(vec_acc1, vec_acc2);
    _mm_store_si128((__m128i *) accumulate, vec_acc1);

    for (int i = stop_len; i < len; i++) {
        tmp_acc += src[i];
    }

    tmp_acc = tmp_acc + accumulate[0] + accumulate[1] + accumulate[2] + accumulate[3];

    *dst = tmp_acc;
}

// Experimental

static inline void copy128s(int32_t *src, int32_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT32;
    stop_len *= SSE_LEN_INT32;

#ifdef OMP
#pragma omp parallel for schedule(auto)
#endif
    for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
        _mm_store_si128((__m128i *) (dst + i), _mm_load_si128((__m128i *) (src + i)));
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = src[i];
    }
}

static inline void copy128s_2(int32_t *src, int32_t *dst, int len)
{
    int stop_len = len / (2 * SSE_LEN_INT32);
    stop_len *= (2 * SSE_LEN_INT32);

#ifdef OMP
#pragma omp parallel for schedule(auto)
#endif
    for (int i = 0; i < stop_len; i += 2 * SSE_LEN_INT32) {
        __m128i tmp1 = _mm_load_si128((__m128i *) (src + i));
        __m128i tmp2 = _mm_load_si128((__m128i *) (src + i + SSE_LEN_INT32));
        _mm_store_si128((__m128i *) (dst + i), tmp1);
        _mm_store_si128((__m128i *) (dst + i + SSE_LEN_INT32), tmp2);
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = src[i];
    }
}

static inline void fast_copy128s(int32_t *src, int32_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT32;
    stop_len *= SSE_LEN_INT32;

#ifdef OMP
#pragma omp parallel for schedule(auto)
#endif
    for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
        _mm_stream_si128((__m128i *) (dst + i), _mm_stream_load_si128((__m128i *) (src + i)));
    }
    _mm_mfence();

    for (int i = stop_len; i < len; i++) {
        dst[i] = src[i];
    }
}


static inline void fast_copy128s_2(int32_t *src, int32_t *dst, int len)
{
    int stop_len = len / (2 * SSE_LEN_INT32);
    stop_len *= (2 * SSE_LEN_INT32);

#ifdef OMP
#pragma omp parallel for schedule(auto)
#endif
    for (int i = 0; i < stop_len; i += 2 * SSE_LEN_INT32) {
        __m128i tmp1 = _mm_stream_load_si128((__m128i *) (src + i));
        __m128i tmp2 = _mm_stream_load_si128((__m128i *) (src + i + SSE_LEN_INT32));
        _mm_stream_si128((__m128i *) (dst + i), tmp1);
        _mm_stream_si128((__m128i *) (dst + i + SSE_LEN_INT32), tmp2);
    }
    _mm_mfence();

    for (int i = stop_len; i < len; i++) {
        dst[i] = src[i];
    }
}

static inline void fast_copy128s_4(int32_t *src, int32_t *dst, int len)
{
    int stop_len = len / (4 * SSE_LEN_INT32);
    stop_len *= (4 * SSE_LEN_INT32);

#ifdef OMP
#pragma omp parallel for schedule(auto)
#endif
    for (int i = 0; i < stop_len; i += 4 * SSE_LEN_INT32) {
        __m128i tmp1 = _mm_stream_load_si128((__m128i *) (src + i));
        __m128i tmp2 = _mm_stream_load_si128((__m128i *) (src + i + SSE_LEN_INT32));
        __m128i tmp3 = _mm_stream_load_si128((__m128i *) (src + i + 2 * SSE_LEN_INT32));
        __m128i tmp4 = _mm_stream_load_si128((__m128i *) (src + i + 3 * SSE_LEN_INT32));
        _mm_stream_si128((__m128i *) (dst + i), tmp1);
        _mm_stream_si128((__m128i *) (dst + i + SSE_LEN_INT32), tmp2);
        _mm_stream_si128((__m128i *) (dst + i + 2 * SSE_LEN_INT32), tmp3);
        _mm_stream_si128((__m128i *) (dst + i + 3 * SSE_LEN_INT32), tmp4);
    }
    _mm_mfence();

    for (int i = stop_len; i < len; i++) {
        dst[i] = src[i];
    }
}


// Adapted from NEON2SSE (does not exists for X86)
static inline __m128i _mm_absdiff_epi16(__m128i a, __m128i b)
{
#ifndef ARM
    __m128i cmp, difab, difba;
    cmp = _mm_cmpgt_epi16(a, b);
    difab = _mm_sub_epi16(a, b);
    difba = _mm_sub_epi16(b, a);
    difab = _mm_and_si128(cmp, difab);
    difba = _mm_andnot_si128(cmp, difba);
    return _mm_or_si128(difab, difba);
#else
    return vreinterpretq_m128i_s16(vabdq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
#endif
}

// Adapted from NEON2SSE (does not exists for X86)
static inline __m128i _mm_absdiff_epi32(__m128i a, __m128i b)
{
#ifndef ARM
    __m128i cmp, difab, difba;
    cmp = _mm_cmpgt_epi32(a, b);
    difab = _mm_sub_epi32(a, b);
    difba = _mm_sub_epi32(b, a);
    difab = _mm_and_si128(cmp, difab);
    difba = _mm_andnot_si128(cmp, difba);
    return _mm_or_si128(difab, difba);
#else
    return vreinterpretq_m128i_s32(vabdq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
#endif
}

static inline __m128i _mm_absdiff_epi8(__m128i a, __m128i b)
{
#ifndef ARM
    __m128i cmp, difab, difba;
    cmp = _mm_cmpgt_epi8(a, b);
    difab = _mm_sub_epi8(a, b);
    difba = _mm_sub_epi8(b, a);
    difab = _mm_and_si128(cmp, difab);
    difba = _mm_andnot_si128(cmp, difba);
    return _mm_or_si128(difab, difba);
#else
    return vreinterpretq_m128i_s8(vabdq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
#endif
}

static inline void absdiff16s_128s(int16_t *src1, int16_t *src2, int16_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT16;
    stop_len *= SSE_LEN_INT16;


    if (areAligned3((uintptr_t) (src1), (uintptr_t) (src2), (uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT16) {
            __m128i a = _mm_load_si128((__m128i *) (src1 + i));
            __m128i b = _mm_load_si128((__m128i *) (src2 + i));
            _mm_store_si128((__m128i *) (dst + i), _mm_absdiff_epi16(a, b));
        }
    } else {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT16) {
            __m128i a = _mm_loadu_si128((__m128i *) (src1 + i));
            __m128i b = _mm_loadu_si128((__m128i *) (src2 + i));
            _mm_storeu_si128((__m128i *) (dst + i), _mm_absdiff_epi16(a, b));
        }
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = abs(src1[i] - src2[i]);
    }
}

/*
static inline void print8i(__m128i v)
{
    int16_t *p = (int16_t *) &v;
#ifndef __SSE2__
    _mm_empty();
#endif
    printf("[%d, %d, %d, %d,%d, %d, %d, %d]", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}*/

static inline void powerspect16s_128s_interleaved(complex16s_t *src, int32_t *dst, int len)
{
    int stop_len = len / SSE_LEN_INT32;
    stop_len *= SSE_LEN_INT32;

    int j = 0;
    if (areAligned2((uintptr_t) (src), (uintptr_t) (dst), SSE_LEN_BYTES)) {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            __m128i reim = _mm_load_si128((__m128i *) ((const int16_t *) src + j));
            // print8i(reim); printf("\n");
            _mm_store_si128((__m128i *) (dst + i), _mm_madd_epi16(reim, reim));
            j += SSE_LEN_INT16;
        }
    } else {
        for (int i = 0; i < stop_len; i += SSE_LEN_INT32) {
            __m128i reim = _mm_loadu_si128((__m128i *) ((const int16_t *) src + j));
            _mm_storeu_si128((__m128i *) (dst + i), _mm_madd_epi16(reim, reim));
            j += SSE_LEN_INT16;
        }
    }

    for (int i = stop_len; i < len; i++) {
        dst[i] = (int32_t) src[i].re * (int32_t) src[i].re + (int32_t) src[i].im * (int32_t) src[i].im;
    }
}
