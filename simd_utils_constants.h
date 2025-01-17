/*
 * Project : SIMD_Utils
 * Version : 0.2.2
 * Author  : JishinMaster
 * Licence : BSD-2
 */

#if defined(SSE) || defined(AVX) || defined(AVX512)
#ifndef ARM
#include <immintrin.h>
#else
// Also includes arm_neon.h
#include "sse2neon_wrapper.h"
#endif
#endif

#ifdef RISCV
#include <riscv_vector.h>
#endif

#ifdef ALTIVEC
#include <altivec.h>
#endif

#ifdef _MSC_VER /* visual c++ */
#define ALIGN16_BEG __declspec(align(16))
#define ALIGN16_END
#define ALIGN32_BEG
#define ALIGN32_END __declspec(align(32))
#define ALIGN64_BEG
#define ALIGN64_END __declspec(align(64))
#else /* gcc,icc, clang */
#define ALIGN16_BEG
#define ALIGN16_END __attribute__((aligned(16)))
#define ALIGN32_BEG
#define ALIGN32_END __attribute__((aligned(32)))
#define ALIGN64_BEG
#define ALIGN64_END __attribute__((aligned(64)))
#endif


static const float FOPI = 1.27323954473516f;
static const float PIO4F = 0.7853981633974483096f;

/* Note, these constants are for a 32-bit significand: */
/*
static const float DP1 = 0.7853851318359375f;
static const float DP2 = 1.30315311253070831298828125e-5f;
static const float DP3 = 3.03855025325309630e-11f;
static const float lossth = 65536.f;
*/

/* These are for a 24-bit significand: */
static const float minus_cephes_DP1 = -0.78515625f;
static const float minus_cephes_DP2 = -2.4187564849853515625e-4f;
static const float minus_cephes_DP3 = -3.77489497744594108e-8f;
static float lossth = 8192.;

static const float T24M1 = 16777215.f;

static const float sincof[] = {-1.9515295891E-4f, 8.3321608736E-3f, -1.6666654611E-1f};
static const float coscof[] = {2.443315711809948E-5f, -1.388731625493765E-3f,
                               4.166664568298827E-2f};

#define SIGN_MASK 0x80000000
static const int32_t sign_mask = SIGN_MASK;
static const int32_t inv_sign_mask = ~SIGN_MASK;

#define INVLN10 0.4342944819032518f  // 0.4342944819f
#define INVLN2 1.4426950408889634f   // 1.44269504089f
#define LN2 0.6931471805599453094172321214581765680755001343602552541206800094f
#define LN2_DIV_LN10 0.3010299956639811952137388947244930267681898814621085413104274611f
#define IMM8_FLIP_VEC 0x1B              // change m128 from abcd to dcba
#define IMM8_LO_HI_VEC 0x1E             // change m128 from abcd to cdab
#define IMM8_PERMUTE_128BITS_LANES 0x1  // reverse abcd efgh to efgh abcd
#define M_PI 3.14159265358979323846

typedef union {
    struct {
        int16_t re;
        int16_t im;
    };
    int16_t c[2];
} complex16s_t;

typedef union {
    struct {
        int32_t re;
        int32_t im;
    };
    int32_t c[2];
} complex32s_t;

typedef union {
    struct {
        float re;
        float im;
    };
    float c[2];
} complex32_t;

typedef union {
    struct {
        double re;
        double im;
    };
    double c[2];
} complex64_t;

typedef enum {
    RndZero,
    RndNear,
    RndFinancial,
} FloatRoundingMode;


#ifdef SSE

#define SSE_LEN_BYTES 16  // Size of SSE lane
#define SSE_LEN_INT16 8   // number of int16 with an SSE lane
#define SSE_LEN_INT32 4   // number of int32 with an SSE lane
#define SSE_LEN_FLOAT 4   // number of float with an SSE lane
#define SSE_LEN_DOUBLE 2  // number of double with an SSE lane

#if defined(ARM)

typedef float32x4_t v4sf;      // vector of 4 float
typedef float32x4x2_t v4sfx2;  // vector of 4 float
typedef uint32x4_t v4su;       // vector of 4 uint32
typedef int32x4_t v4si;        // vector of 4 uint32
typedef float32x4x2_t v4sfx2;

#define _PS_CONST(Name, Val) \
    static const ALIGN16_BEG float _ps_##Name[4] ALIGN16_END = {Val, Val, Val, Val}
#define _PI32_CONST(Name, Val) \
    static const ALIGN16_BEG int _pi32_##Name[4] ALIGN16_END = {Val, Val, Val, Val}
#define _PS_CONST_TYPE(Name, Type, Val) \
    static const ALIGN16_BEG Type _ps_##Name[4] ALIGN16_END = {Val, Val, Val, Val}

#define c_inv_mant_mask ~0x7f800000u
#define c_cephes_SQRTHF 0.707106781186547524
#define c_cephes_log_p0 7.0376836292E-2
#define c_cephes_log_p1 -1.1514610310E-1
#define c_cephes_log_p2 1.1676998740E-1
#define c_cephes_log_p3 -1.2420140846E-1
#define c_cephes_log_p4 +1.4249322787E-1
#define c_cephes_log_p5 -1.6668057665E-1
#define c_cephes_log_p6 +2.0000714765E-1
#define c_cephes_log_p7 -2.4999993993E-1
#define c_cephes_log_p8 +3.3333331174E-1
#define c_cephes_log_q1 -2.12194440e-4
#define c_cephes_log_q2 0.693359375

#define c_exp_hi 88.3762626647949f
#define c_exp_lo -88.3762626647949f

#define c_cephes_LOG2EF 1.44269504088896341
#define c_cephes_exp_C1 0.693359375
#define c_cephes_exp_C2 -2.12194440e-4

#define c_cephes_exp_p0 1.9875691500E-4
#define c_cephes_exp_p1 1.3981999507E-3
#define c_cephes_exp_p2 8.3334519073E-3
#define c_cephes_exp_p3 4.1665795894E-2
#define c_cephes_exp_p4 1.6666665459E-1
#define c_cephes_exp_p5 5.0000001201E-1

#define c_minus_cephes_DP1 -0.78515625
#define c_minus_cephes_DP2 -2.4187564849853515625e-4
#define c_minus_cephes_DP3 -3.77489497744594108e-8
#define c_sincof_p0 -1.9515295891E-4
#define c_sincof_p1 8.3321608736E-3
#define c_sincof_p2 -1.6666654611E-1
#define c_coscof_p0 2.443315711809948E-005
#define c_coscof_p1 -1.388731625493765E-003
#define c_coscof_p2 4.166664568298827E-002
#define c_cephes_FOPI 1.27323954473516  // 4 / M_PI

#else

typedef __m128 v4sf;    // vector of 4 float (sse1)
typedef __m128i v4si;   // vector of 4 int (sse2)
typedef __m128i v2sid;  // vector of 2 int64 (sse2)
typedef struct {
    v4sf val[2];
} v4sfx2;


#define _PS_CONST(Name, Val) \
    static const ALIGN16_BEG float _ps_##Name[4] ALIGN16_END = {Val, Val, Val, Val}
#define _PI32_CONST(Name, Val) \
    static const ALIGN16_BEG int _pi32_##Name[4] ALIGN16_END = {Val, Val, Val, Val}
#define _PS_CONST_TYPE(Name, Type, Val) \
    static const ALIGN16_BEG Type _ps_##Name[4] ALIGN16_END = {Val, Val, Val, Val}

#endif

#define ROUNDTONEAREST (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)
#define ROUNDTOFLOOR (_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC)
#define ROUNDTOCEIL (_MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)
#define ROUNDTOZERO (_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC)

typedef __m128d v2sd;  // vector of 2 double (sse)
typedef __m128i v2si;  // vector of 2 int 64 (sse)

// Warning, declared in reverse order since it's little endian :
//  const v4sf conj_mask = _mm_set_ps(-1.0f, 1.0f, -1.0f, 1.0f);
static const float _ps_conj_mask[4] __attribute__((aligned(16))) = {1.0f, -1.0f, 1.0f, -1.0f};

#define _PD_CONST(Name, Val) \
    static const ALIGN16_BEG double _pd_##Name[2] ALIGN16_END = {Val, Val}
#define _PI64_CONST(Name, Val) \
    static const ALIGN16_BEG int64_t _pi64_##Name[2] ALIGN16_END = {Val, Val}
#define _PD_CONST_TYPE(Name, Type, Val) \
    static const ALIGN16_BEG Type _pd_##Name[2] ALIGN16_END = {Val, Val}

/////////////// INT //////////////////
_PI32_CONST(1, 1);
_PI32_CONST(inv1, ~1);
_PI32_CONST(2, 2);
_PI32_CONST(4, 4);
_PI32_CONST(0x7f, 0x7f);
_PI64_CONST(1, 1);
_PI64_CONST(inv1, ~1);
_PI64_CONST(2, 2);
_PI64_CONST(4, 4);
_PI64_CONST(0x7f, 0x7f);


/////////////// SINGLE //////////////////
_PS_CONST(1, 1.0f);
_PS_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PS_CONST_TYPE(sign_mask, int, (int) 0x80000000);
_PS_CONST_TYPE(inv_sign_mask, int, ~0x80000000);

_PS_CONST(cephes_SQRTHF, 0.707106781186547524f);
_PS_CONST(cephes_log_p0, 7.0376836292E-2f);
_PS_CONST(cephes_log_p1, -1.1514610310E-1f);
_PS_CONST(cephes_log_p2, 1.1676998740E-1f);
_PS_CONST(cephes_log_p3, -1.2420140846E-1f);
_PS_CONST(cephes_log_p4, +1.4249322787E-1f);
_PS_CONST(cephes_log_p5, -1.6668057665E-1f);
_PS_CONST(cephes_log_p6, +2.0000714765E-1f);
_PS_CONST(cephes_log_p7, -2.4999993993E-1f);
_PS_CONST(cephes_log_p8, +3.3333331174E-1f);
_PS_CONST(cephes_log_q1, -2.12194440e-4f);
_PS_CONST(cephes_log_q2, 0.693359375f);

_PS_CONST(exp_hi, 88.3762626647949f);
_PS_CONST(exp_lo, -88.3762626647949f);

_PS_CONST(cephes_LOG2EF, 1.44269504088896341f);
_PS_CONST(cephes_exp_C1, 0.693359375f);
_PS_CONST(cephes_exp_C2, -2.12194440e-4f);

_PS_CONST(cephes_exp_p0, 1.9875691500E-4f);
_PS_CONST(cephes_exp_p1, 1.3981999507E-3f);
_PS_CONST(cephes_exp_p2, 8.3334519073E-3f);
_PS_CONST(cephes_exp_p3, 4.1665795894E-2f);
_PS_CONST(cephes_exp_p4, 1.6666665459E-1f);
_PS_CONST(cephes_exp_p5, 5.0000001201E-1f);

_PS_CONST(minus_cephes_DP1, -0.78515625f);
_PS_CONST(minus_cephes_DP2, -2.4187564849853515625e-4f);
_PS_CONST(minus_cephes_DP3, -3.77489497744594108e-8f);
_PS_CONST(sincof_p0, -1.9515295891E-4f);
_PS_CONST(sincof_p1, 8.3321608736E-3f);
_PS_CONST(sincof_p2, -1.6666654611E-1f);
_PS_CONST(coscof_p0, 2.443315711809948E-005f);
_PS_CONST(coscof_p1, -1.388731625493765E-003f);
_PS_CONST(coscof_p2, 4.166664568298827E-002f);
_PS_CONST(cephes_FOPI, 1.27323954473516f);  // 4 / M_PI

_PS_CONST(min1, -1.0f);
_PS_CONST(min2, -2.0f);
_PS_CONST(min0p5, -0.5f);

// For tanf
_PS_CONST(DP123, 0.78515625 + 2.4187564849853515625e-4 + 3.77489497744594108e-8);

// Neg values to better migrate to FMA
_PS_CONST(DP1, -0.78515625f);
_PS_CONST(DP2, -2.4187564849853515625E-4f);
_PS_CONST(DP3, -3.77489497744594108E-8f);

_PS_CONST(FOPI, 1.27323954473516f); /* 4/pi */
_PS_CONST(TAN_P0, 9.38540185543E-3f);
_PS_CONST(TAN_P1, 3.11992232697E-3f);
_PS_CONST(TAN_P2, 2.44301354525E-2f);
_PS_CONST(TAN_P3, 5.34112807005E-2f);
_PS_CONST(TAN_P4, 1.33387994085E-1f);
_PS_CONST(TAN_P5, 3.33331568548E-1f);

_PS_CONST(ASIN_P0, 4.2163199048E-2f);
_PS_CONST(ASIN_P1, 2.4181311049E-2f);
_PS_CONST(ASIN_P2, 4.5470025998E-2f);
_PS_CONST(ASIN_P3, 7.4953002686E-2f);
_PS_CONST(ASIN_P4, 1.6666752422E-1f);

_PS_CONST(PIF, 3.14159265358979323846f);      // PI
_PS_CONST(mPIF, -3.14159265358979323846f);    // -PI
_PS_CONST(PIO2F, 1.57079632679489661923f);    // PI/2 1.570796326794896619
_PS_CONST(mPIO2F, -1.57079632679489661923f);  // -PI/2 1.570796326794896619
_PS_CONST(PIO4F, 0.785398163397448309615f);   // PI/4 0.7853981633974483096

_PS_CONST(TANPI8F, 0.414213562373095048802f);   // tan(pi/8) => 0.4142135623730950
_PS_CONST(TAN3PI8F, 2.414213562373095048802f);  // tan(3*pi/8) => 2.414213562373095

_PS_CONST(ATAN_P0, 8.05374449538e-2f);
_PS_CONST(ATAN_P1, -1.38776856032E-1f);
_PS_CONST(ATAN_P2, 1.99777106478E-1f);
_PS_CONST(ATAN_P3, -3.33329491539E-1f);

_PS_CONST_TYPE(pos_sign_mask, int, (int) 0x7FFFFFFF);
_PS_CONST_TYPE(neg_sign_mask, int, (int) ~0x7FFFFFFF);

_PS_CONST(MAXLOGF, 88.72283905206835f);
_PS_CONST(MAXLOGFDIV2, 44.361419526034176f);
_PS_CONST(MINLOGF, -103.278929903431851103f);
_PS_CONST(cephes_exp_minC1, -0.693359375f);
_PS_CONST(cephes_exp_minC2, 2.12194440e-4f);

_PS_CONST(0p625, 0.625f);
_PS_CONST(TANH_P0, -5.70498872745E-3f);
_PS_CONST(TANH_P1, 2.06390887954E-2f);
_PS_CONST(TANH_P2, -5.37397155531E-2f);
_PS_CONST(TANH_P3, 1.33314422036E-1f);
_PS_CONST(TANH_P4, -3.33332819422E-1f);

_PS_CONST(MAXNUMF, 3.4028234663852885981170418348451692544e38f);
_PS_CONST(minMAXNUMF, -3.4028234663852885981170418348451692544e38f);
_PS_CONST(SINH_P0, 2.03721912945E-4f);
_PS_CONST(SINH_P1, 8.33028376239E-3f);
_PS_CONST(SINH_P2, 1.66667160211E-1f);

_PS_CONST(1emin4, 1e-4f);
_PS_CONST(ATANH_P0, 1.81740078349E-1f);
_PS_CONST(ATANH_P1, 8.24370301058E-2f);
_PS_CONST(ATANH_P2, 1.46691431730E-1f);
_PS_CONST(ATANH_P3, 1.99782164500E-1f);
_PS_CONST(ATANH_P4, 3.33337300303E-1f);

_PS_CONST_TYPE(zero, int, (int) 0x00000000);
_PS_CONST(1500, 1500.0f);
_PS_CONST(LOGE2F, 0.693147180559945309f);
_PS_CONST(ASINH_P0, 2.0122003309E-2f);
_PS_CONST(ASINH_P1, -4.2699340972E-2f);
_PS_CONST(ASINH_P2, 7.4847586088E-2f);
_PS_CONST(ASINH_P3, -1.6666288134E-1f);

_PS_CONST(ACOSH_P0, 1.7596881071E-3f);
_PS_CONST(ACOSH_P1, -7.5272886713E-3f);
_PS_CONST(ACOSH_P2, 2.6454905019E-2f);
_PS_CONST(ACOSH_P3, -1.1784741703E-1f);
_PS_CONST(ACOSH_P4, 1.4142135263E0f);

/* For log10f */
_PS_CONST(cephes_L102A, 3.0078125E-1f);
_PS_CONST(cephes_L102B, 2.48745663981195213739E-4f);
_PS_CONST(cephes_L10EA, 4.3359375E-1f);
_PS_CONST(cephes_L10EB, 7.00731903251827651129E-4f);

/* For log2f */
_PS_CONST(cephes_LOG2EA, 0.44269504088896340735992f);

/////////////// DOUBLE //////////////////
_PD_CONST_TYPE(zero, int, (int) 0x00000000);
_PD_CONST_TYPE(min_norm_pos, int64_t, 0x380ffff83ce549caL);
_PD_CONST_TYPE(mant_mask, int64_t, 0xFFFFFFFFFFFFFL);
_PD_CONST_TYPE(inv_mant_mask, int64_t, ~0xFFFFFFFFFFFFFL);
_PD_CONST_TYPE(sign_mask, int64_t, (int64_t) 0x8000000000000000L);
_PD_CONST_TYPE(inv_sign_mask, int64_t, ~0x8000000000000000L);

_PD_CONST(minus_cephes_DP1, -7.85398125648498535156E-1);
_PD_CONST(minus_cephes_DP2, -3.77489470793079817668E-8);
_PD_CONST(minus_cephes_DP3, -2.69515142907905952645E-15);
_PD_CONST(sincof_p0, 1.58962301576546568060E-10);
_PD_CONST(sincof_p1, -2.50507477628578072866E-8);
_PD_CONST(sincof_p2, 2.75573136213857245213E-6);
_PD_CONST(sincof_p3, -1.98412698295895385996E-4);
_PD_CONST(sincof_p4, 8.33333333332211858878E-3);
_PD_CONST(sincof_p5, -1.66666666666666307295E-1);
_PD_CONST(coscof_p0, -1.13585365213876817300E-11);
_PD_CONST(coscof_p1, 2.08757008419747316778E-9);
_PD_CONST(coscof_p2, -2.75573141792967388112E-7);
_PD_CONST(coscof_p3, 2.48015872888517045348E-5);
_PD_CONST(coscof_p4, -1.38888888888730564116E-3);
_PD_CONST(coscof_p5, 4.16666666666665929218E-2);
_PD_CONST(cephes_FOPI, 1.2732395447351626861510701069801148);  // 4 / M_PI

_PD_CONST(1, 1.0);
_PD_CONST(2, 2.0);
_PD_CONST(0p5, 0.5);

_PD_CONST(cephes_SQRTHF, 0.70710678118654752440);
_PD_CONST(cephes_log_p0, 1.01875663804580931796E-4);
_PD_CONST(cephes_log_p1, -4.97494994976747001425E-1);
_PD_CONST(cephes_log_p2, 4.70579119878881725854E0);
_PD_CONST(cephes_log_p3, -1.44989225341610930846E1);
_PD_CONST(cephes_log_p4, +1.79368678507819816313E1);
_PD_CONST(cephes_log_p5, -7.70838733755885391666E0);

_PD_CONST(cephes_log_q1, -1.12873587189167450590E1);
_PD_CONST(cephes_log_q2, 4.52279145837532221105E1);
_PD_CONST(cephes_log_q3, -8.29875266912776603211E1);
_PD_CONST(cephes_log_q4, 7.11544750618563894466E1);
_PD_CONST(cephes_log_q5, 4.52279145837532221105E1);
_PD_CONST(cephes_log_q6, -2.31251620126765340583E1);

_PD_CONST(exp_hi, 709.437);
_PD_CONST(exp_lo, -709.436139303);

_PD_CONST(cephes_LOG2EF, 1.4426950408889634073599);

_PD_CONST(cephes_exp_p0, 1.26177193074810590878e-4);
_PD_CONST(cephes_exp_p1, 3.02994407707441961300e-2);
_PD_CONST(cephes_exp_p2, 9.99999999999999999910e-1);

_PD_CONST(cephes_exp_q0, 3.00198505138664455042e-6);
_PD_CONST(cephes_exp_q1, 2.52448340349684104192e-3);
_PD_CONST(cephes_exp_q2, 2.27265548208155028766e-1);
_PD_CONST(cephes_exp_q3, 2.00000000000000000009e0);

_PD_CONST(cephes_exp_C1, 0.693145751953125);
_PD_CONST(cephes_exp_C2, 1.42860682030941723212e-6);

_PD_CONST_TYPE(positive_mask, int64_t, (int64_t) 0x7FFFFFFFFFFFFFFFL);
_PD_CONST_TYPE(negative_mask, int64_t, (int64_t) ~0x7FFFFFFFFFFFFFFFL);
_PD_CONST(ASIN_P0, 4.253011369004428248960E-3);
_PD_CONST(ASIN_P1, -6.019598008014123785661E-1);
_PD_CONST(ASIN_P2, 5.444622390564711410273E0);
_PD_CONST(ASIN_P3, -1.626247967210700244449E1);
_PD_CONST(ASIN_P4, 1.956261983317594739197E1);
_PD_CONST(ASIN_P5, -8.198089802484824371615E0);

_PD_CONST(ASIN_Q0, -1.474091372988853791896E1);
_PD_CONST(ASIN_Q1, 7.049610280856842141659E1);
_PD_CONST(ASIN_Q2, -1.471791292232726029859E2);
_PD_CONST(ASIN_Q3, 1.395105614657485689735E2);
_PD_CONST(ASIN_Q4, -4.918853881490881290097E1);

_PD_CONST(ASIN_R0, 2.967721961301243206100E-3);
_PD_CONST(ASIN_R1, -5.634242780008963776856E-1);
_PD_CONST(ASIN_R2, 6.968710824104713396794E0);
_PD_CONST(ASIN_R3, -2.556901049652824852289E1);
_PD_CONST(ASIN_R4, 2.853665548261061424989E1);

_PD_CONST(ASIN_S0, -2.194779531642920639778E1);
_PD_CONST(ASIN_S1, 1.470656354026814941758E2);
_PD_CONST(ASIN_S2, -3.838770957603691357202E2);
_PD_CONST(ASIN_S3, 3.424398657913078477438E2);

_PD_CONST(PIO2, 1.57079632679489661923);    /* pi/2 */
_PD_CONST(PIO4, 7.85398163397448309616E-1); /* pi/4 */

_PD_CONST(minMOREBITS, -6.123233995736765886130E-17);
_PD_CONST(MOREBITS, 6.123233995736765886130E-17);

_PD_CONST(ATAN_P0, -8.750608600031904122785E-1);
_PD_CONST(ATAN_P1, -1.615753718733365076637E1);
_PD_CONST(ATAN_P2, -7.500855792314704667340E1);
_PD_CONST(ATAN_P3, -1.228866684490136173410E2);
_PD_CONST(ATAN_P4, -6.485021904942025371773E1);

_PD_CONST(ATAN_Q0, 2.485846490142306297962E1);
_PD_CONST(ATAN_Q1, 1.650270098316988542046E2);
_PD_CONST(ATAN_Q2, 4.328810604912902668951E2);
_PD_CONST(ATAN_Q3, 4.853903996359136964868E2);
_PD_CONST(ATAN_Q4, 1.945506571482613964425E2);

_PD_CONST(TAN3PI8, 2.41421356237309504880); /* 3*pi/8 */

_PD_CONST(min1, -1.0);

#endif


#ifdef AVX

#define AVX_LEN_BYTES 32  // Size of AVX lane
#define AVX_LEN_INT16 16  // number of int16 with an AVX lane
#define AVX_LEN_INT32 8   // number of int32 with an AVX lane
#define AVX_LEN_FLOAT 8   // number of float with an AVX lane
#define AVX_LEN_DOUBLE 4  // number of double with an AVX lane

#define _PI32AVX_CONST(Name, Val) \
    static const ALIGN32_BEG int _pi32avx_##Name[4] ALIGN32_END = {Val, Val, Val, Val}
#define _PS256_CONST(Name, Val) \
    static const ALIGN32_BEG float _ps256_##Name[8] ALIGN32_END = {Val, Val, Val, Val, Val, Val, Val, Val}
#define _PI32_CONST256(Name, Val) \
    static const ALIGN32_BEG int _pi32_256_##Name[8] ALIGN32_END = {Val, Val, Val, Val, Val, Val, Val, Val}
#define _PS256_CONST_TYPE(Name, Type, Val) \
    static const ALIGN32_BEG Type _ps256_##Name[8] ALIGN32_END = {Val, Val, Val, Val, Val, Val, Val, Val}
#define _PD256_CONST(Name, Val) \
    static const ALIGN32_BEG double _pd256_##Name[4] ALIGN32_END = {Val, Val, Val, Val}
#define _PI256_64_CONST(Name, Val) \
    static const ALIGN32_BEG int64_t _pi256_64_##Name[4] ALIGN32_END = {Val, Val, Val, Val}
#define _PD256_CONST_TYPE(Name, Type, Val) \
    static const ALIGN32_BEG Type _pd256_##Name[4] ALIGN32_END = {Val, Val, Val, Val}

static const float _ps256_conj_mask[8] __attribute__((aligned(32))) = {1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f};

typedef __m256 v8sf;    // vector of 8 float (avx)
typedef __m256i v8si;   // vector of 8 int   (avx)
typedef __m256i v4sid;  // vector of 4 64 bits int   (avx)
typedef __m128i v4si;   // vector of 4 int   (avx)
typedef __m256d v4sd;   // vector of 4 double (avx)
typedef struct {
    v8sf val[2];
} v8sfx2;

#ifndef __AVX2__

typedef union imm_xmm_union {
    v8si imm;
    v4si xmm[2];
} imm_xmm_union;

#define COPY_IMM_TO_XMM(imm_, xmm0_, xmm1_)           \
    {                                                 \
        imm_xmm_union u __attribute__((aligned(32))); \
        u.imm = imm_;                                 \
        xmm0_ = u.xmm[0];                             \
        xmm1_ = u.xmm[1];                             \
    }

#define COPY_XMM_TO_IMM(xmm0_, xmm1_, imm_)           \
    {                                                 \
        imm_xmm_union u __attribute__((aligned(32))); \
        u.xmm[0] = xmm0_;                             \
        u.xmm[1] = xmm1_;                             \
        imm_ = u.imm;                                 \
    }

#define AVX2_BITOP_USING_SSE2(fn)                            \
    static inline v8si _mm256_##fn(v8si x, int a)            \
    {                                                        \
        /* use SSE2 instruction to perform the bitop AVX2 */ \
        v4si x1, x2;                                         \
        v8si ret;                                            \
        COPY_IMM_TO_XMM(x, x1, x2);                          \
        x1 = _mm_##fn(x1, a);                                \
        x2 = _mm_##fn(x2, a);                                \
        COPY_XMM_TO_IMM(x1, x2, ret);                        \
        return (ret);                                        \
    }

#warning "Using SSE2 to perform AVX2 bitshift ops"
AVX2_BITOP_USING_SSE2(slli_epi32)
AVX2_BITOP_USING_SSE2(srli_epi32)

#define AVX2_INTOP_USING_SSE2(fn)                                         \
    static inline v8si _mm256_##fn(v8si x, v8si y)                        \
    {                                                                     \
        /* use SSE2 instructions to perform the AVX2 integer operation */ \
        v4si x1, x2;                                                      \
        v4si y1, y2;                                                      \
        v8si ret;                                                         \
        COPY_IMM_TO_XMM(x, x1, x2);                                       \
        COPY_IMM_TO_XMM(y, y1, y2);                                       \
        x1 = _mm_##fn(x1, y1);                                            \
        x2 = _mm_##fn(x2, y2);                                            \
        COPY_XMM_TO_IMM(x1, x2, ret);                                     \
        return (ret);                                                     \
    }

#warning "Using SSE2 to perform AVX2 integer ops"
AVX2_INTOP_USING_SSE2(and_si128)
AVX2_INTOP_USING_SSE2(andnot_si128)
AVX2_INTOP_USING_SSE2(cmpeq_epi32)
AVX2_INTOP_USING_SSE2(sub_epi32)
AVX2_INTOP_USING_SSE2(add_epi32)

#endif /* __AVX2__ */

/////////////// INT //////////////////
_PI32AVX_CONST(1, 1);
_PI32AVX_CONST(inv1, ~1);
_PI32AVX_CONST(2, 2);
_PI32AVX_CONST(4, 4);

_PI32_CONST256(0, 0);
_PI32_CONST256(1, 1);
_PI32_CONST256(inv1, ~1);
_PI32_CONST256(2, 2);
_PI32_CONST256(4, 4);
_PI32_CONST256(0x7f, 0x7f);

_PI256_64_CONST(1, 1);
_PI256_64_CONST(inv1, ~1);
_PI256_64_CONST(2, 2);
_PI256_64_CONST(4, 4);
_PI256_64_CONST(0x7f, 0x7f);


/////////////// SINGLE //////////////////
_PS256_CONST(1, 1.0f);
_PS256_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS256_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS256_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS256_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PS256_CONST_TYPE(sign_mask, int, (int) 0x80000000);
_PS256_CONST_TYPE(inv_sign_mask, int, ~0x80000000);

_PS256_CONST(cephes_SQRTHF, 0.707106781186547524f);
_PS256_CONST(cephes_log_p0, 7.0376836292E-2f);
_PS256_CONST(cephes_log_p1, -1.1514610310E-1f);
_PS256_CONST(cephes_log_p2, 1.1676998740E-1f);
_PS256_CONST(cephes_log_p3, -1.2420140846E-1f);
_PS256_CONST(cephes_log_p4, +1.4249322787E-1f);
_PS256_CONST(cephes_log_p5, -1.6668057665E-1f);
_PS256_CONST(cephes_log_p6, +2.0000714765E-1f);
_PS256_CONST(cephes_log_p7, -2.4999993993E-1f);
_PS256_CONST(cephes_log_p8, +3.3333331174E-1f);
_PS256_CONST(cephes_log_q1, -2.12194440e-4f);
_PS256_CONST(cephes_log_q2, 0.693359375f);

_PS256_CONST(exp_hi, 88.3762626647949f);
_PS256_CONST(exp_lo, -88.3762626647949f);

_PS256_CONST(cephes_LOG2EF, 1.44269504088896341f);
_PS256_CONST(cephes_exp_C1, 0.693359375f);
_PS256_CONST(cephes_exp_C2, -2.12194440e-4f);

_PS256_CONST(cephes_exp_p0, 1.9875691500E-4f);
_PS256_CONST(cephes_exp_p1, 1.3981999507E-3f);
_PS256_CONST(cephes_exp_p2, 8.3334519073E-3f);
_PS256_CONST(cephes_exp_p3, 4.1665795894E-2f);
_PS256_CONST(cephes_exp_p4, 1.6666665459E-1f);
_PS256_CONST(cephes_exp_p5, 5.0000001201E-1f);

_PS256_CONST(minus_cephes_DP1, -0.78515625f);
_PS256_CONST(minus_cephes_DP2, -2.4187564849853515625e-4f);
_PS256_CONST(minus_cephes_DP3, -3.77489497744594108e-8f);
_PS256_CONST(sincof_p0, -1.9515295891E-4f);
_PS256_CONST(sincof_p1, 8.3321608736E-3f);
_PS256_CONST(sincof_p2, -1.6666654611E-1f);
_PS256_CONST(coscof_p0, 2.443315711809948E-005f);
_PS256_CONST(coscof_p1, -1.388731625493765E-003f);
_PS256_CONST(coscof_p2, 4.166664568298827E-002f);
_PS256_CONST(cephes_FOPI, 1.27323954473516f);  // 4 / M_PI

_PS256_CONST(min1, -1.0f);
_PS256_CONST(min2, -2.0f);
_PS256_CONST(min0p5, -0.5f);

// For tanf
_PS256_CONST(DP123, 0.78515625 + 2.4187564849853515625e-4 + 3.77489497744594108e-8);

// Neg values to better migrate to FMA
_PS256_CONST(DP1, -0.78515625);
_PS256_CONST(DP2, -2.4187564849853515625e-4);
_PS256_CONST(DP3, -3.77489497744594108e-8);

_PS256_CONST(FOPI, 1.27323954473516); /* 4/pi */
_PS256_CONST(TAN_P0, 9.38540185543E-3);
_PS256_CONST(TAN_P1, 3.11992232697E-3);
_PS256_CONST(TAN_P2, 2.44301354525E-2);
_PS256_CONST(TAN_P3, 5.34112807005E-2);
_PS256_CONST(TAN_P4, 1.33387994085E-1);
_PS256_CONST(TAN_P5, 3.33331568548E-1);

_PS256_CONST(ASIN_P0, 4.2163199048E-2);
_PS256_CONST(ASIN_P1, 2.4181311049E-2);
_PS256_CONST(ASIN_P2, 4.5470025998E-2);
_PS256_CONST(ASIN_P3, 7.4953002686E-2);
_PS256_CONST(ASIN_P4, 1.6666752422E-1);

_PS256_CONST(PIF, 3.14159265358979323846);      // PI
_PS256_CONST(mPIF, -3.14159265358979323846);    // -PI
_PS256_CONST(PIO2F, 1.57079632679489661923);    // PI/2 1.570796326794896619
_PS256_CONST(mPIO2F, -1.57079632679489661923);  // -PI/2 1.570796326794896619
_PS256_CONST(PIO4F, 0.785398163397448309615);   // PI/4 0.7853981633974483096

_PS256_CONST(TANPI8F, 0.414213562373095048802);   // tan(pi/8) => 0.4142135623730950
_PS256_CONST(TAN3PI8F, 2.414213562373095048802);  // tan(3*pi/8) => 2.414213562373095

_PS256_CONST(ATAN_P0, 8.05374449538e-2);
_PS256_CONST(ATAN_P1, -1.38776856032E-1);
_PS256_CONST(ATAN_P2, 1.99777106478E-1);
_PS256_CONST(ATAN_P3, -3.33329491539E-1);

_PS256_CONST_TYPE(pos_sign_mask, int, (int) 0x7FFFFFFF);
_PS256_CONST_TYPE(neg_sign_mask, int, (int) ~0x7FFFFFFF);

_PS256_CONST(MAXLOGF, 88.72283905206835f);
_PS256_CONST(MAXLOGFDIV2, 44.361419526034176f);
_PS256_CONST(MINLOGF, -103.278929903431851103f);
_PS256_CONST(cephes_exp_minC1, -0.693359375f);
_PS256_CONST(cephes_exp_minC2, 2.12194440e-4f);
_PS256_CONST(0p625, 0.625f);

_PS256_CONST(TANH_P0, -5.70498872745E-3f);
_PS256_CONST(TANH_P1, 2.06390887954E-2f);
_PS256_CONST(TANH_P2, -5.37397155531E-2f);
_PS256_CONST(TANH_P3, 1.33314422036E-1f);
_PS256_CONST(TANH_P4, -3.33332819422E-1f);

_PS256_CONST(MAXNUMF, 3.4028234663852885981170418348451692544e38f);
_PS256_CONST(minMAXNUMF, -3.4028234663852885981170418348451692544e38f);
_PS256_CONST(SINH_P0, 2.03721912945E-4f);
_PS256_CONST(SINH_P1, 8.33028376239E-3f);
_PS256_CONST(SINH_P2, 1.66667160211E-1f);

_PS256_CONST(1emin4, 1e-4f);
_PS256_CONST(ATANH_P0, 1.81740078349E-1f);
_PS256_CONST(ATANH_P1, 8.24370301058E-2f);
_PS256_CONST(ATANH_P2, 1.46691431730E-1f);
_PS256_CONST(ATANH_P3, 1.99782164500E-1f);
_PS256_CONST(ATANH_P4, 3.33337300303E-1f);

_PS256_CONST(1500, 1500.0f);
_PS256_CONST(LOGE2F, 0.693147180559945309f);
_PS256_CONST(ASINH_P0, 2.0122003309E-2f);
_PS256_CONST(ASINH_P1, -4.2699340972E-2f);
_PS256_CONST(ASINH_P2, 7.4847586088E-2f);
_PS256_CONST(ASINH_P3, -1.6666288134E-1f);

_PS256_CONST(ACOSH_P0, 1.7596881071E-3f);
_PS256_CONST(ACOSH_P1, -7.5272886713E-3f);
_PS256_CONST(ACOSH_P2, 2.6454905019E-2f);
_PS256_CONST(ACOSH_P3, -1.1784741703E-1f);
_PS256_CONST(ACOSH_P4, 1.4142135263E0f);

/* For log10f */
_PS256_CONST(cephes_L102A, 3.0078125E-1f);
_PS256_CONST(cephes_L102B, 2.48745663981195213739E-4f);
_PS256_CONST(cephes_L10EA, 4.3359375E-1f);
_PS256_CONST(cephes_L10EB, 7.00731903251827651129E-4f);

/* For log2f */
_PS256_CONST(cephes_LOG2EA, 0.44269504088896340735992f);

/////////////// DOUBLE //////////////////
_PD256_CONST_TYPE(min_norm_pos, int64_t, 0x380ffff83ce549caL);
_PD256_CONST_TYPE(mant_mask, int64_t, 0xFFFFFFFFFFFFFL);
_PD256_CONST_TYPE(inv_mant_mask, int64_t, ~0xFFFFFFFFFFFFFL);
_PD256_CONST_TYPE(sign_mask, int64_t, (int64_t) 0x8000000000000000L);
_PD256_CONST_TYPE(inv_sign_mask, int64_t, ~0x8000000000000000L);

_PD256_CONST(minus_cephes_DP1, -7.85398125648498535156E-1);
_PD256_CONST(minus_cephes_DP2, -3.77489470793079817668E-8);
_PD256_CONST(minus_cephes_DP3, -2.69515142907905952645E-15);
_PD256_CONST(sincof_p0, 1.58962301576546568060E-10);
_PD256_CONST(sincof_p1, -2.50507477628578072866E-8);
_PD256_CONST(sincof_p2, 2.75573136213857245213E-6);
_PD256_CONST(sincof_p3, -1.98412698295895385996E-4);
_PD256_CONST(sincof_p4, 8.33333333332211858878E-3);
_PD256_CONST(sincof_p5, -1.66666666666666307295E-1);
_PD256_CONST(coscof_p0, -1.13585365213876817300E-11);
_PD256_CONST(coscof_p1, 2.08757008419747316778E-9);
_PD256_CONST(coscof_p2, -2.75573141792967388112E-7);
_PD256_CONST(coscof_p3, 2.48015872888517045348E-5);
_PD256_CONST(coscof_p4, -1.38888888888730564116E-3);
_PD256_CONST(coscof_p5, 4.16666666666665929218E-2);
_PD256_CONST(cephes_FOPI, 1.2732395447351626861510701069801148);  // 4 / M_PI

_PD256_CONST_TYPE(positive_mask, int64_t, (int64_t) 0x7FFFFFFFFFFFFFFFL);
_PD256_CONST_TYPE(negative_mask, int64_t, (int64_t) ~0x7FFFFFFFFFFFFFFFL);
_PD256_CONST(ASIN_P0, 4.253011369004428248960E-3);
_PD256_CONST(ASIN_P1, -6.019598008014123785661E-1);
_PD256_CONST(ASIN_P2, 5.444622390564711410273E0);
_PD256_CONST(ASIN_P3, -1.626247967210700244449E1);
_PD256_CONST(ASIN_P4, 1.956261983317594739197E1);
_PD256_CONST(ASIN_P5, -8.198089802484824371615E0);

_PD256_CONST(ASIN_Q0, -1.474091372988853791896E1);
_PD256_CONST(ASIN_Q1, 7.049610280856842141659E1);
_PD256_CONST(ASIN_Q2, -1.471791292232726029859E2);
_PD256_CONST(ASIN_Q3, 1.395105614657485689735E2);
_PD256_CONST(ASIN_Q4, -4.918853881490881290097E1);

_PD256_CONST(ASIN_R0, 2.967721961301243206100E-3);
_PD256_CONST(ASIN_R1, -5.634242780008963776856E-1);
_PD256_CONST(ASIN_R2, 6.968710824104713396794E0);
_PD256_CONST(ASIN_R3, -2.556901049652824852289E1);
_PD256_CONST(ASIN_R4, 2.853665548261061424989E1);

_PD256_CONST(ASIN_S0, -2.194779531642920639778E1);
_PD256_CONST(ASIN_S1, 1.470656354026814941758E2);
_PD256_CONST(ASIN_S2, -3.838770957603691357202E2);
_PD256_CONST(ASIN_S3, 3.424398657913078477438E2);

_PD256_CONST(PIO2, 1.57079632679489661923);    /* pi/2 */
_PD256_CONST(PIO4, 7.85398163397448309616E-1); /* pi/4 */

_PD256_CONST(minMOREBITS, -6.123233995736765886130E-17);
_PD256_CONST(MOREBITS, 6.123233995736765886130E-17);

_PD256_CONST(ATAN_P0, -8.750608600031904122785E-1);
_PD256_CONST(ATAN_P1, -1.615753718733365076637E1);
_PD256_CONST(ATAN_P2, -7.500855792314704667340E1);
_PD256_CONST(ATAN_P3, -1.228866684490136173410E2);
_PD256_CONST(ATAN_P4, -6.485021904942025371773E1);

_PD256_CONST(ATAN_Q0, 2.485846490142306297962E1);
_PD256_CONST(ATAN_Q1, 1.650270098316988542046E2);
_PD256_CONST(ATAN_Q2, 4.328810604912902668951E2);
_PD256_CONST(ATAN_Q3, 4.853903996359136964868E2);
_PD256_CONST(ATAN_Q4, 1.945506571482613964425E2);

_PD256_CONST(TAN3PI8, 2.41421356237309504880); /* 3*pi/8 */

_PD256_CONST(min1, -1.0);
_PD256_CONST(1, 1.0);
_PD256_CONST(2, 2.0);
_PD256_CONST(0p5, 0.5);

#endif


#ifdef AVX512

#define AVX512_LEN_BYTES 64  // Size of AVX512 lane
#define AVX512_LEN_INT16 32  // number of int16 with an AVX512 lane
#define AVX512_LEN_INT32 16  // number of int32 with an AVX512 lane
#define AVX512_LEN_FLOAT 16  // number of float with an AVX512 lane
#define AVX512_LEN_DOUBLE 8  // number of double with an AVX512 lane

typedef __m512 v16sf;   // vector of 16 float (avx512)
typedef __m512i v16si;  // vector of 16 int   (avx512)
typedef __m512i v8sid;  // vector of 8 64bits int   (avx512)
typedef __m256i v8si;   // vector of 8 int   (avx)
typedef __m512d v8sd;   // vector of 8 double (avx512)
typedef struct {
    v16sf val[2];
} v16sfx2;

#define _PI64AVX512_CONST(Name, Val) \
    static const ALIGN64_BEG int _pi64avx_##Name[8] ALIGN64_END = {Val, Val, Val, Val, Val, Val, Val, Val}

/* declare some AVX512 constants */
#define _PS512_CONST(Name, Val) \
    static const ALIGN64_BEG float _ps512_##Name[16] ALIGN64_END = {Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val}
#define _PI32_CONST512(Name, Val) \
    static const ALIGN64_BEG int _pi32_512_##Name[16] ALIGN64_END = {Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val}
#define _PS512_CONST_TYPE(Name, Type, Val) \
    static const ALIGN64_BEG Type _ps512_##Name[16] ALIGN64_END = {Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val, Val}
#define _PD512_CONST(Name, Val) \
    static const ALIGN64_BEG double _pd512_##Name[8] ALIGN64_END = {Val, Val, Val, Val, Val, Val, Val, Val}
#define _PI512_64_CONST(Name, Val) \
    static const ALIGN64_BEG int64_t _pi512_64_##Name[8] ALIGN64_END = {Val, Val, Val, Val, Val, Val, Val, Val}
#define _PD512_CONST_TYPE(Name, Type, Val) \
    static const ALIGN64_BEG Type _pd512_##Name[8] ALIGN64_END = {Val, Val, Val, Val, Val, Val, Val, Val}

////////// INT /////////////
_PI32_CONST512(0, 0);
_PI32_CONST512(1, 1);
_PI32_CONST512(inv1, ~1);
_PI32_CONST512(2, 2);
_PI32_CONST512(4, 4);
_PI32_CONST512(0x7f, 0x7f);
_PI512_64_CONST(1, 1);
_PI512_64_CONST(inv1, ~1);
_PI512_64_CONST(2, 2);
_PI512_64_CONST(4, 4);
_PI512_64_CONST(0x7f, 0x7f);
_PI64AVX512_CONST(1, 1);
_PI64AVX512_CONST(inv1, ~1);
_PI64AVX512_CONST(2, 2);
_PI64AVX512_CONST(4, 4);

// used for cplxtoreal transforms

// Select alternatively indexes between Real and Complex Elements of the two 512bit vectors
//  indexes with 0x1X means second vector argument and X position (hexa)
static const int _pi32_512_idx_re[16] __attribute__((aligned(64))) = {0x10, 0x12, 0x14, 0x16,
                                                                      0x18, 0x1A, 0x1C, 0x1E, 0, 2, 4, 6, 8, 10, 12, 14};
static const int _pi32_512_idx_im[16] __attribute__((aligned(64))) = {0x11, 0x13, 0x15, 0x17,
                                                                      0x19, 0x1B, 0x1D, 0x1F, 1, 3, 5, 7, 9, 11, 13, 15};
// used for realtocplx transforms
static const int _pi32_512_idx_cplx_lo[16] __attribute__((aligned(64))) = {0x10, 0, 0x11, 1,
                                                                           0x12, 2, 0x13, 3, 0x14, 4, 0x15, 5, 0x16, 6, 0x17, 7};
static const int _pi32_512_idx_cplx_hi[16] __attribute__((aligned(64))) = {0x18, 8, 0x19, 9,
                                                                           0x1A, 10, 0x1B, 11, 0x1C, 12, 0x1D, 13, 0x1E, 14, 0x1F, 15};

////////// SINGLE /////////////
_PS512_CONST(1, 1.0f);
_PS512_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS512_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS512_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS512_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PS512_CONST_TYPE(sign_mask, int, (int) 0x80000000);
_PS512_CONST_TYPE(inv_sign_mask, int, ~0x80000000);
_PS512_CONST(cephes_SQRTHF, 0.707106781186547524f);
_PS512_CONST(cephes_log_p0, 7.0376836292E-2f);
_PS512_CONST(cephes_log_p1, -1.1514610310E-1f);
_PS512_CONST(cephes_log_p2, 1.1676998740E-1f);
_PS512_CONST(cephes_log_p3, -1.2420140846E-1f);
_PS512_CONST(cephes_log_p4, +1.4249322787E-1f);
_PS512_CONST(cephes_log_p5, -1.6668057665E-1f);
_PS512_CONST(cephes_log_p6, +2.0000714765E-1f);
_PS512_CONST(cephes_log_p7, -2.4999993993E-1f);
_PS512_CONST(cephes_log_p8, +3.3333331174E-1f);
_PS512_CONST(cephes_log_q1, -2.12194440e-4f);
_PS512_CONST(cephes_log_q2, 0.693359375f);

_PS512_CONST(exp_hi, 88.3762626647949f);
_PS512_CONST(exp_lo, -88.3762626647949f);

_PS512_CONST(cephes_LOG2EF, 1.44269504088896341f);
_PS512_CONST(cephes_exp_C1, 0.693359375f);
_PS512_CONST(cephes_exp_C2, -2.12194440e-4f);

_PS512_CONST(cephes_exp_p0, 1.9875691500E-4f);
_PS512_CONST(cephes_exp_p1, 1.3981999507E-3f);
_PS512_CONST(cephes_exp_p2, 8.3334519073E-3f);
_PS512_CONST(cephes_exp_p3, 4.1665795894E-2f);
_PS512_CONST(cephes_exp_p4, 1.6666665459E-1f);
_PS512_CONST(cephes_exp_p5, 5.0000001201E-1f);

_PS512_CONST(minus_cephes_DP1, -0.78515625f);
_PS512_CONST(minus_cephes_DP2, -2.4187564849853515625e-4f);
_PS512_CONST(minus_cephes_DP3, -3.77489497744594108e-8f);
_PS512_CONST(sincof_p0, -1.9515295891E-4f);
_PS512_CONST(sincof_p1, 8.3321608736E-3f);
_PS512_CONST(sincof_p2, -1.6666654611E-1f);
_PS512_CONST(coscof_p0, 2.443315711809948E-005f);
_PS512_CONST(coscof_p1, -1.388731625493765E-003f);
_PS512_CONST(coscof_p2, 4.166664568298827E-002f);
_PS512_CONST(cephes_FOPI, 1.27323954473516f);  // 4 / M_PI

_PS512_CONST(min1, -1.0f);
_PS512_CONST(plus1, 1.0f);
_PS512_CONST(min2, -2.0f);
_PS512_CONST(min0p5, -0.5f);

// For tanf
_PS512_CONST(DP123, 0.78515625 + 2.4187564849853515625e-4 + 3.77489497744594108e-8);

// Neg values to better migrate to FMA
_PS512_CONST(DP1, -0.78515625f);
_PS512_CONST(DP2, -2.4187564849853515625e-4f);
_PS512_CONST(DP3, -3.77489497744594108e-8f);

_PS512_CONST(FOPI, 1.27323954473516f); /* 4/pi */
_PS512_CONST(TAN_P0, 9.38540185543E-3f);
_PS512_CONST(TAN_P1, 3.11992232697E-3f);
_PS512_CONST(TAN_P2, 2.44301354525E-2f);
_PS512_CONST(TAN_P3, 5.34112807005E-2f);
_PS512_CONST(TAN_P4, 1.33387994085E-1f);
_PS512_CONST(TAN_P5, 3.33331568548E-1f);

_PS512_CONST(ASIN_P0, 4.2163199048E-2f);
_PS512_CONST(ASIN_P1, 2.4181311049E-2f);
_PS512_CONST(ASIN_P2, 4.5470025998E-2f);
_PS512_CONST(ASIN_P3, 7.4953002686E-2f);
_PS512_CONST(ASIN_P4, 1.6666752422E-1f);

_PS512_CONST(PIF, 3.14159265358979323846f);      // PI
_PS512_CONST(mPIF, -3.14159265358979323846f);    // -PI
_PS512_CONST(PIO2F, 1.57079632679489661923f);    // PI/2 1.570796326794896619
_PS512_CONST(mPIO2F, -1.57079632679489661923f);  // -PI/2 1.570796326794896619
_PS512_CONST(PIO4F, 0.785398163397448309615f);   // PI/4 0.7853981633974483096

_PS512_CONST(TANPI8F, 0.414213562373095048802f);   // tan(pi/8) => 0.4142135623730950
_PS512_CONST(TAN3PI8F, 2.414213562373095048802f);  // tan(3*pi/8) => 2.414213562373095

_PS512_CONST(ATAN_P0, 8.05374449538e-2f);
_PS512_CONST(ATAN_P1, -1.38776856032E-1f);
_PS512_CONST(ATAN_P2, 1.99777106478E-1f);
_PS512_CONST(ATAN_P3, -3.33329491539E-1f);

_PS512_CONST_TYPE(pos_sign_mask, int, (int) 0x7FFFFFFF);
_PS512_CONST_TYPE(neg_sign_mask, int, (int) ~0x7FFFFFFF);

_PS512_CONST(MAXLOGF, 88.72283905206835f);
_PS512_CONST(MAXLOGFDIV2, 44.361419526034176f);
_PS512_CONST(0p625, 0.625f);
_PS512_CONST(TANH_P0, -5.70498872745E-3f);
_PS512_CONST(TANH_P1, 2.06390887954E-2f);
_PS512_CONST(TANH_P2, -5.37397155531E-2f);
_PS512_CONST(TANH_P3, 1.33314422036E-1f);
_PS512_CONST(TANH_P4, -3.33332819422E-1f);

_PS512_CONST(MAXNUMF, 3.4028234663852885981170418348451692544e38f);
_PS512_CONST(minMAXNUMF, -3.4028234663852885981170418348451692544e38f);
_PS512_CONST(SINH_P0, 2.03721912945E-4f);
_PS512_CONST(SINH_P1, 8.33028376239E-3f);
_PS512_CONST(SINH_P2, 1.66667160211E-1f);

_PS512_CONST(1emin4, 1e-4f);
_PS512_CONST(ATANH_P0, 1.81740078349E-1f);
_PS512_CONST(ATANH_P1, 8.24370301058E-2f);
_PS512_CONST(ATANH_P2, 1.46691431730E-1f);
_PS512_CONST(ATANH_P3, 1.99782164500E-1f);
_PS512_CONST(ATANH_P4, 3.33337300303E-1f);

_PS512_CONST(1500, 1500.0f);
_PS512_CONST(LOGE2F, 0.693147180559945309f);
_PS512_CONST(ASINH_P0, 2.0122003309E-2f);
_PS512_CONST(ASINH_P1, -4.2699340972E-2f);
_PS512_CONST(ASINH_P2, 7.4847586088E-2f);
_PS512_CONST(ASINH_P3, -1.6666288134E-1f);

_PS512_CONST(ACOSH_P0, 1.7596881071E-3f);
_PS512_CONST(ACOSH_P1, -7.5272886713E-3f);
_PS512_CONST(ACOSH_P2, 2.6454905019E-2f);
_PS512_CONST(ACOSH_P3, -1.1784741703E-1f);
_PS512_CONST(ACOSH_P4, 1.4142135263E0f);

/* For log10f */
_PS512_CONST(cephes_L102A, 3.0078125E-1f);
_PS512_CONST(cephes_L102B, 2.48745663981195213739E-4f);
_PS512_CONST(cephes_L10EA, 4.3359375E-1f);
_PS512_CONST(cephes_L10EB, 7.00731903251827651129E-4f);

/* For log2f */
_PS512_CONST(cephes_LOG2EA, 0.44269504088896340735992f);

////////// DOUBLE /////////////
_PD512_CONST_TYPE(min_norm_pos, int64_t, 0x380ffff83ce549caL);
_PD512_CONST_TYPE(mant_mask, int64_t, 0xFFFFFFFFFFFFFL);
_PD512_CONST_TYPE(inv_mant_mask, int64_t, ~0xFFFFFFFFFFFFFL);
_PD512_CONST_TYPE(sign_mask, int64_t, (int64_t) 0x8000000000000000L);
_PD512_CONST_TYPE(inv_sign_mask, int64_t, ~0x8000000000000000L);

_PD512_CONST(minus_cephes_DP1, -7.85398125648498535156E-1);
_PD512_CONST(minus_cephes_DP2, -3.77489470793079817668E-8);
_PD512_CONST(minus_cephes_DP3, -2.69515142907905952645E-15);
_PD512_CONST(sincof_p0, 1.58962301576546568060E-10);
_PD512_CONST(sincof_p1, -2.50507477628578072866E-8);
_PD512_CONST(sincof_p2, 2.75573136213857245213E-6);
_PD512_CONST(sincof_p3, -1.98412698295895385996E-4);
_PD512_CONST(sincof_p4, 8.33333333332211858878E-3);
_PD512_CONST(sincof_p5, -1.66666666666666307295E-1);
_PD512_CONST(coscof_p0, -1.13585365213876817300E-11);
_PD512_CONST(coscof_p1, 2.08757008419747316778E-9);
_PD512_CONST(coscof_p2, -2.75573141792967388112E-7);
_PD512_CONST(coscof_p3, 2.48015872888517045348E-5);
_PD512_CONST(coscof_p4, -1.38888888888730564116E-3);
_PD512_CONST(coscof_p5, 4.16666666666665929218E-2);
_PD512_CONST(cephes_FOPI, 1.2732395447351626861510701069801148);  // 4 / M_PI

_PD512_CONST_TYPE(positive_mask, int64_t, (int64_t) 0x7FFFFFFFFFFFFFFFL);
_PD512_CONST_TYPE(negative_mask, int64_t, (int64_t) ~0x7FFFFFFFFFFFFFFFL);
_PD512_CONST(ASIN_P0, 4.253011369004428248960E-3);
_PD512_CONST(ASIN_P1, -6.019598008014123785661E-1);
_PD512_CONST(ASIN_P2, 5.444622390564711410273E0);
_PD512_CONST(ASIN_P3, -1.626247967210700244449E1);
_PD512_CONST(ASIN_P4, 1.956261983317594739197E1);
_PD512_CONST(ASIN_P5, -8.198089802484824371615E0);

_PD512_CONST(ASIN_Q0, -1.474091372988853791896E1);
_PD512_CONST(ASIN_Q1, 7.049610280856842141659E1);
_PD512_CONST(ASIN_Q2, -1.471791292232726029859E2);
_PD512_CONST(ASIN_Q3, 1.395105614657485689735E2);
_PD512_CONST(ASIN_Q4, -4.918853881490881290097E1);

_PD512_CONST(ASIN_R0, 2.967721961301243206100E-3);
_PD512_CONST(ASIN_R1, -5.634242780008963776856E-1);
_PD512_CONST(ASIN_R2, 6.968710824104713396794E0);
_PD512_CONST(ASIN_R3, -2.556901049652824852289E1);
_PD512_CONST(ASIN_R4, 2.853665548261061424989E1);

_PD512_CONST(ASIN_S0, -2.194779531642920639778E1);
_PD512_CONST(ASIN_S1, 1.470656354026814941758E2);
_PD512_CONST(ASIN_S2, -3.838770957603691357202E2);
_PD512_CONST(ASIN_S3, 3.424398657913078477438E2);

_PD512_CONST(PIO2, 1.57079632679489661923);    /* pi/2 */
_PD512_CONST(PIO4, 7.85398163397448309616E-1); /* pi/4 */

_PD512_CONST(minMOREBITS, -6.123233995736765886130E-17);
_PD512_CONST(MOREBITS, 6.123233995736765886130E-17);

_PD512_CONST(ATAN_P0, -8.750608600031904122785E-1);
_PD512_CONST(ATAN_P1, -1.615753718733365076637E1);
_PD512_CONST(ATAN_P2, -7.500855792314704667340E1);
_PD512_CONST(ATAN_P3, -1.228866684490136173410E2);
_PD512_CONST(ATAN_P4, -6.485021904942025371773E1);

_PD512_CONST(ATAN_Q0, 2.485846490142306297962E1);
_PD512_CONST(ATAN_Q1, 1.650270098316988542046E2);
_PD512_CONST(ATAN_Q2, 4.328810604912902668951E2);
_PD512_CONST(ATAN_Q3, 4.853903996359136964868E2);
_PD512_CONST(ATAN_Q4, 1.945506571482613964425E2);

_PD512_CONST(TAN3PI8, 2.41421356237309504880); /* 3*pi/8 */

_PD512_CONST(min1, -1.0);
_PD512_CONST(1, 1.0);
_PD512_CONST(2, 2.0);
_PD512_CONST(0p5, 0.5);

#endif


#ifdef ALTIVEC

#define ALTIVEC_LEN_FLOAT 4
#define ALTIVEC_LEN_BYTES 16

#define ALIGN16_BEG
#define ALIGN16_END __attribute__((aligned(16)))

#define _PS_CONST(Name, Val) \
    static const ALIGN16_BEG float _ps_##Name[4] ALIGN16_END = {Val, Val, Val, Val}
#define _PI32_CONST(Name, Val) \
    static const ALIGN16_BEG int _pi32_##Name[4] ALIGN16_END = {Val, Val, Val, Val}
#define _PS_CONST_TYPE(Name, Type, Val) \
    static const ALIGN16_BEG Type _ps_##Name[4] ALIGN16_END = {Val, Val, Val, Val}
#define _PI8_CONST(Name, Val)                                                                           \
    static const ALIGN16_BEG unsigned char _pi8_##Name[16] ALIGN16_END = {Val, Val, Val, Val, Val, Val, \
                                                                          Val, Val, Val, Val, Val, Val, Val, Val, Val, Val}

_PI8_CONST(0, 0x00);
_PS_CONST(0, 0.0f);
_PS_CONST(1, 1.0f);
_PS_CONST(0p5, 0.5f);
_PS_CONST(min1, -1.0f);
_PI8_CONST(ff, 0xFF);

typedef __vector float v4sf;
typedef __vector int v4si;
typedef __vector unsigned char v16u8;
typedef __vector char v16s8;
#endif

/// PRINT FUNCTIONS */
/*

static inline void print4(__m128 v)
{
    float *p = (float *) &v;
#ifndef __SSE2__
    _mm_empty();
#endif
    printf("[%3.24g, %3.24g, %3.24g, %3.24g]", p[0], p[1], p[2], p[3]);
}

static inline void print4short(__m64 v)
{
    uint16_t *p = (uint16_t *) &v;
#ifndef __SSE2__
    _mm_empty();
#endif
    printf("[%u, %u, %u, %u]", p[0], p[1], p[2], p[3]);
}

static inline void print2(__m128d v)
{
    double *p = (double *) &v;
#ifndef USE_SSE2
    _mm_empty();
#endif
    printf("[%13.8g, %13.8g]", p[0], p[1]);
}

static inline void print2i(__m128i v)
{
    int64_t *p = (int64_t *) &v;
#ifndef USE_SSE2
    _mm_empty();
#endif
    printf("[%ld, %ld]", p[0], p[1]);
}

static inline void print8(__m256 v)
{
    float *p = (float *) &v;
#ifndef __SSE2__
    _mm_empty();
#endif
    printf("[%3.5g, %3.5g, %3.5g, %3.5g, %3.5g, %3.5g, %3.5g, %3.5g]", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

#ifdef __AVX2__

static inline void print4d(__m256d v)
{
    double *p = (double *) &v;
#ifndef USE_SSE2
    _mm_empty();
#endif
    printf("[%13.8g, %13.8g %13.8g, %13.8g]", p[0], p[1], p[2], p[3]);
}

static inline void print4id(__m256i v)
{
    int64_t *p = (int64_t *) &v;
#ifndef USE_SSE2
    _mm_empty();
#endif
    printf("[%ld, %ld %ld, %ld]", p[0], p[1], p[2], p[3]);
}

static inline void print8d(__m512d v)
{
    double *p = (double *) &v;
#ifndef USE_SSE2
    _mm_empty();
#endif
    printf("[%13.8g, %13.8g %13.8g, %13.8g %13.8g, %13.8g %13.8g, %13.8g ]", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

*/
