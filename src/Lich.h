#ifndef LICH_H
#define LICH_H

#include <stdint.h>
#include <string.h>

#define LICH_PI     3.14159265358979323846f
#define LICH_TAU    (LICH_PI * 2.0f)
#define LICH_PI_2   (LICH_PI * 0.5f)
#define LICH_1_TAU  (1.0f / LICH_TAU)

#ifndef LICH_INLINE
#define LICH_INLINE static inline
#endif
#define LICH_PURE
#define LICH_LIKELY(x)   (x)
#define LICH_UNLIKELY(x) (x)

LICH_INLINE float lich__bits_para_float(unsigned int i) {
    float f; memcpy(&f, &i, sizeof(f)); return f;
}
LICH_INLINE unsigned int lich__float_para_bits(float f) {
    unsigned int i; memcpy(&i, &f, sizeof(i)); return i;
}
LICH_INLINE int          lich__eh_nan(float v)                 { return v != v; }

LICH_INLINE LICH_PURE float lich_absf(float v) {
    return lich__bits_para_float(lich__float_para_bits(v) & 0x7FFFFFFFu);
}

LICH_INLINE LICH_PURE float lich_minf(float a, float b)              { return a < b ? a : b; }
LICH_INLINE LICH_PURE float lich_maxf(float a, float b)              { return a > b ? a : b; }
LICH_INLINE LICH_PURE float lich_clampf(float v, float mn, float mx) { return lich_minf(lich_maxf(v, mn), mx); }

LICH_INLINE LICH_PURE float lich_floorf(float v) {
    if (LICH_UNLIKELY(lich_absf(v) >= 8388608.0f)) return v;
    int i = (int)v;
    return (float)(i - (v < (float)i));
}

LICH_INLINE LICH_PURE float lich_ceilf(float v) {
    if (LICH_UNLIKELY(lich_absf(v) >= 8388608.0f)) return v;
    int i = (int)v;
    return (float)(i + (v > (float)i));
}

LICH_INLINE LICH_PURE float lich_roundf(float v) {
    return v >= 0.0f ? lich_floorf(v + 0.5f) : lich_ceilf(v - 0.5f);
}

LICH_INLINE LICH_PURE float lich_truncf(float v) {
    if (LICH_UNLIKELY(lich_absf(v) >= 8388608.0f)) return v;
    return (float)(int)v;
}

LICH_INLINE LICH_PURE float lich_lerpf(float a, float b, float t) { return a + t * (b - a); }

LICH_INLINE LICH_PURE float lich_fmodf(float x, float y) {
    if (LICH_UNLIKELY(y == 0.0f || lich__eh_nan(x) || lich__eh_nan(y))) return 0.0f;
    return x - lich_truncf(x / y) * y;
}

LICH_INLINE LICH_PURE float lich_rsqrtf(float v) {
    float half = 0.5f * v;
    unsigned int i = 0x5F375A86u - (lich__float_para_bits(v) >> 1);
    float y = lich__bits_para_float(i);
    y *= 1.5f - half * y * y;
    y *= 1.5f - half * y * y;
    return y;
}

LICH_INLINE LICH_PURE float lich_sqrtf(float v) {
    if (LICH_UNLIKELY(v <= 0.0f)) return 0.0f;
    float half = 0.5f * v;
    unsigned int i = 0x5F375A86u - (lich__float_para_bits(v) >> 1);
    float y = lich__bits_para_float(i);
    y *= 1.5f - half * y * y;
    y *= 1.5f - half * y * y;
    return v * y;
}

#ifndef LICH_LUT_BITS
#  define LICH_LUT_BITS 9
#endif

#define LICH_LUT_TAMANHO (1 << LICH_LUT_BITS)
#define LICH_LUT_MASCARA (LICH_LUT_TAMANHO - 1)

#ifdef LICH_EXTERN_LUT
extern const float lich__lut_sin [LICH_LUT_TAMANHO + 1];
extern const float lich__lut_log2[LICH_LUT_TAMANHO + 1];
extern const float lich__lut_exp2[LICH_LUT_TAMANHO + 1];
extern const float lich__lut_atan[LICH_LUT_TAMANHO + 1];
#else

static float lich__lut_sin [LICH_LUT_TAMANHO + 1];
static float lich__lut_log2[LICH_LUT_TAMANHO + 1];
static float lich__lut_exp2[LICH_LUT_TAMANHO + 1];
static float lich__lut_atan[LICH_LUT_TAMANHO + 1];
static int   lich__lut_pronta = 0;

LICH_INLINE float lich__init_sin(float x) {
    float x2 = x * x;
    return x * (1.0f + x2 * (-0.16666667f
               + x2 * ( 0.00833333f
               + x2 * (-0.00019841f
               + x2 *   0.00000276f))));
}

LICH_INLINE float lich__init_cos(float x) {
    float x2 = x * x;
    return 1.0f + x2 * (-0.50000000f
           + x2 * ( 0.04166667f
           + x2 * (-0.00138889f
           + x2 *   0.00002480f)));
}

LICH_INLINE float lich__init_log2(float v) {
    unsigned int i = lich__float_para_bits(v);
    i = (i & 0x007FFFFFu) | 0x3F800000u;
    float m = lich__bits_para_float(i) - 1.0f;
    return m * (1.4426950f + m * (-0.7213475f + m * (0.4808983f + m * (-0.3606737f + m * 0.2885390f))));
}

LICH_INLINE float lich__init_exp2(float t) {
    return 1.0f + t * (0.6931472f + t * (0.2402265f + t * (0.0555041f + t * 0.0096181f)));
}

LICH_INLINE float lich__init_atan(float x) {
    float x2 = x * x;
    return x * (1.0f + x2 * (-0.33333333f
               + x2 * ( 0.19999753f
               + x2 * (-0.14211724f
               + x2 *   0.10643544f))));
}

LICH_INLINE void lich_lut_init(void) {
    if (lich__lut_pronta) return;
    for (int n = 0; n <= LICH_LUT_TAMANHO; n++) {
        float t = (float)n / (float)LICH_LUT_TAMANHO;
        lich__lut_sin [n] = lich__init_sin (t * LICH_PI_2);
        lich__lut_log2[n] = lich__init_log2(1.0f + t);
        lich__lut_exp2[n] = lich__init_exp2(t);
        lich__lut_atan[n] = lich__init_atan(t);
    }
    lich__lut_pronta = 1;
}

#define LICH__GUARDA_LUT() do { if (!lich__lut_pronta) lich_lut_init(); } while(0)

#endif

LICH_INLINE float lich__lut_consulta(const float* lut, float t) {
    float fi = t * (float)LICH_LUT_TAMANHO;
    int   lo = (int)fi;
    float fr = fi - (float)lo;
    if (lo < 0)                 { lo = 0;                    fr = 0.0f; }
    if (lo >= LICH_LUT_TAMANHO) { lo = LICH_LUT_TAMANHO - 1; fr = 1.0f; }
    return lut[lo] + fr * (lut[lo + 1] - lut[lo]);
}

LICH_INLINE float lich__log2f_lut(float v) {
    LICH__GUARDA_LUT();
    if (LICH_UNLIKELY(v <= 0.0f)) return -1e38f;
    unsigned int bits     = lich__float_para_bits(v);
    unsigned int bits_exp = (bits >> 23) & 0xFFu;
    int          exp      = (int)bits_exp - 127;
    float        mant     = lich__bits_para_float((bits & 0x007FFFFFu) | 0x3F800000u);
    float        t        = mant - 1.0f;
    return (float)exp + lich__lut_consulta(lich__lut_log2, t);
}

LICH_INLINE float lich__exp2f_lut(float v) {
    LICH__GUARDA_LUT();
    if (LICH_UNLIKELY(lich__eh_nan(v) || v < -126.0f)) return 0.0f;
    if (LICH_UNLIKELY(v > 127.0f)) return lich__bits_para_float(0x7F800000u);
    int          ei       = (int)v;
    float        ef       = v - (float)ei;
    if (ef < 0.0f) { ei--; ef += 1.0f; }
    unsigned int deslocado = (unsigned int)(ei + 127) << 23;
    return lich__bits_para_float(deslocado) * lich__lut_consulta(lich__lut_exp2, ef);
}

LICH_INLINE LICH_PURE float lich_powf(float a, float b) {
    if (LICH_UNLIKELY(b == 0.0f)) return 1.0f;
    if (LICH_UNLIKELY(a <= 0.0f)) return 0.0f;
    return lich__exp2f_lut(b * lich__log2f_lut(a));
}

LICH_INLINE float lich__wrap_pi(float ang) {
    return ang - LICH_TAU * lich_roundf(ang * LICH_1_TAU);
}

LICH_INLINE float lich__sinf_lut(float x) {
    int neg = (x < 0.0f);
    if (neg) x = -x;
    float t = x * (2.0f / LICH_PI);
    LICH__GUARDA_LUT();
    float s = lich__lut_consulta(lich__lut_sin, t);
    return neg ? -s : s;
}

LICH_INLINE float lich__cosf_lut(float x) {
    LICH__GUARDA_LUT();
    float t = (LICH_PI_2 - x) * (2.0f / LICH_PI);
    return lich__lut_consulta(lich__lut_sin, t);
}

LICH_INLINE LICH_PURE float lich_sinf(float ang) {
    ang = lich__wrap_pi(ang);
    if      (ang >  LICH_PI_2) ang =  LICH_PI - ang;
    else if (ang < -LICH_PI_2) ang = -LICH_PI - ang;
    return lich__sinf_lut(ang);
}

LICH_INLINE LICH_PURE float lich_cosf(float ang) {
    ang = lich__wrap_pi(ang);
    if (ang < 0.0f) ang = -ang;
    int neg = (ang > LICH_PI_2);
    if (neg) ang = LICH_PI - ang;
    float c = lich__cosf_lut(ang);
    return neg ? -c : c;
}

LICH_INLINE void lich_sincosf(float ang, float* s, float* c) {
    ang = lich__wrap_pi(ang);
    float sa = ang;
    if      (sa >  LICH_PI_2) sa =  LICH_PI - sa;
    else if (sa < -LICH_PI_2) sa = -LICH_PI - sa;
    float ca   = ang < 0.0f ? -ang : ang;
    int   cneg = (ca > LICH_PI_2);
    if (cneg) ca = LICH_PI - ca;
    if (s) *s = lich__sinf_lut(sa);
    if (c) { float cv = lich__cosf_lut(ca); *c = cneg ? -cv : cv; }
}

LICH_INLINE LICH_PURE float lich_tanf(float ang) {
    float s, c;
    lich_sincosf(ang, &s, &c);
    if (LICH_UNLIKELY(lich_absf(c) < 1e-6f)) return s >= 0.0f ? 1e38f : -1e38f;
    return s / c;
}

LICH_INLINE LICH_PURE float lich_atanf(float x) {
    int neg = (x < 0.0f); if (neg) x = -x;
    int inv = (x > 1.0f); if (inv) x = 1.0f / x;
    LICH__GUARDA_LUT();
    float r = lich__lut_consulta(lich__lut_atan, x);
    if (inv) r = LICH_PI_2 - r;
    return neg ? -r : r;
}

LICH_INLINE LICH_PURE float lich_atan2f(float y, float x) {
    if (LICH_UNLIKELY(x == 0.0f)) {
        if (y > 0.0f) return  LICH_PI_2;
        if (y < 0.0f) return -LICH_PI_2;
        return 0.0f;
    }
    float r = lich_atanf(y / x);
    if (x < 0.0f) r += (y >= 0.0f) ? LICH_PI : -LICH_PI;
    return r;
}

#define LICH_FP_DESLOCAMENTO  16
#define LICH_FP_UM            ((int32_t)65536)
#define LICH_FP_MEIO          ((int32_t)32768)
#define LICH_FP_INT(x)        ((x) >> LICH_FP_DESLOCAMENTO)
#define LICH_FP_FRAC(x)       ((x) & (int32_t)0xFFFF)

LICH_INLINE int32_t lich_fp_de_float(float v)      { return (int32_t)(v * (float)LICH_FP_UM); }
LICH_INLINE int32_t lich_fp_de_frac(float v)       { return (int32_t)((v - lich_floorf(v)) * (float)LICH_FP_UM); }
LICH_INLINE float   lich_fp_para_float(int32_t fp) { return (float)fp * (1.0f / (float)LICH_FP_UM); }
LICH_INLINE int32_t lich_fp_soma(int32_t a, int32_t b)  { return a + b; }
LICH_INLINE int32_t lich_fp_sub(int32_t a, int32_t b)   { return a - b; }
LICH_INLINE int32_t lich_fp_mul(int32_t a, int32_t b)   { return (int32_t)(((int64_t)a * b) >> LICH_FP_DESLOCAMENTO); }

LICH_INLINE int32_t lich_fp_div(int32_t a, int32_t b) {
    if (LICH_UNLIKELY(b == 0)) return (a >= 0) ? (int32_t)0x7FFFFFFF : (int32_t)0x80000001;
    return (int32_t)(((int64_t)a << LICH_FP_DESLOCAMENTO) / (int64_t)b);
}

LICH_INLINE int32_t lich_fp_lerp(int32_t a, int32_t b, int32_t t) { return a + lich_fp_mul(b - a, t); }
LICH_INLINE int32_t lich_fp_floor(int32_t fp)  { return fp & ~(int32_t)0xFFFF; }
LICH_INLINE int32_t lich_fp_ceil(int32_t fp)   { int32_t f = LICH_FP_FRAC(fp); return f ? (lich_fp_floor(fp) + LICH_FP_UM) : fp; }
LICH_INLINE int32_t lich_fp_round(int32_t fp)  { return lich_fp_floor(fp + LICH_FP_MEIO); }
LICH_INLINE int32_t lich_fp_frac(int32_t fp)   { return fp & (int32_t)0xFFFF; }
LICH_INLINE int32_t lich_fp_abs(int32_t fp)    { int32_t m = fp >> 31; return (fp ^ m) - m; }
LICH_INLINE int32_t lich_fp_min(int32_t a, int32_t b)                { return a < b ? a : b; }
LICH_INLINE int32_t lich_fp_max(int32_t a, int32_t b)                { return a > b ? a : b; }
LICH_INLINE int32_t lich_fp_clamp(int32_t v, int32_t mn, int32_t mx) { return lich_fp_min(lich_fp_max(v, mn), mx); }
LICH_INLINE int32_t lich_fp_wrap_pot2(int32_t fp, int32_t mascara)   { return fp & mascara; }

LICH_INLINE int32_t lich_fp_mod(int32_t fp, int32_t periodo) {
    if (LICH_UNLIKELY(periodo <= 0)) return 0;
    int32_t r = fp % periodo;
    return r < 0 ? r + periodo : r;
}

LICH_INLINE int32_t  lich_iabs(int32_t v)                               { int32_t m = v >> 31; return (v ^ m) - m; }
LICH_INLINE int32_t  lich_imin(int32_t a, int32_t b)                    { return a < b ? a : b; }
LICH_INLINE int32_t  lich_imax(int32_t a, int32_t b)                    { return a > b ? a : b; }
LICH_INLINE int32_t  lich_iclamp(int32_t v, int32_t mn, int32_t mx)     { return lich_imin(lich_imax(v, mn), mx); }
LICH_INLINE int32_t  lich_lerp_i(int32_t a, int32_t b, int32_t t)       { return a + (((b - a) * t) >> 8); }

LICH_INLINE uint32_t lich_isqrt(uint32_t n) {
    if (n == 0) return 0;
    uint32_t x = n, y = (x + 1u) >> 1;
    while (y < x) { x = y; y = (x + n / x) >> 1; }
    return x;
}

LICH_INLINE int lich_ilog2(uint32_t n) {
    int r = 0;
    if (n == 0) return -1;
    if (n >= (1u << 16)) { n >>= 16; r += 16; }
    if (n >= (1u <<  8)) { n >>=  8; r +=  8; }
    if (n >= (1u <<  4)) { n >>=  4; r +=  4; }
    if (n >= (1u <<  2)) { n >>=  2; r +=  2; }
    if (n >= (1u <<  1)) {           r +=  1; }
    return r;
}

LICH_INLINE int      lich_eh_pot2(uint32_t n)   { return (n > 0u) && !(n & (n - 1u)); }

LICH_INLINE uint32_t lich_prox_pot2(uint32_t n) {
    if (n == 0u) return 1u;
    n--;
    n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16;
    return n + 1u;
}

LICH_INLINE uint32_t lich_udiv_round(uint32_t a, uint32_t b) {
    if (LICH_UNLIKELY(b == 0u)) return 0u;
    return (a + b / 2u) / b;
}

LICH_INLINE LICH_PURE float lich_v2_dot   (float ax, float ay, float bx, float by) { return ax*bx + ay*by; }
LICH_INLINE LICH_PURE float lich_v2_cross (float ax, float ay, float bx, float by) { return ax*by - ay*bx; }
LICH_INLINE LICH_PURE float lich_v2_len2  (float x,  float y)                      { return x*x + y*y; }
LICH_INLINE LICH_PURE float lich_v2_len   (float x,  float y)                      { return lich_sqrtf(x*x + y*y); }
LICH_INLINE LICH_PURE float lich_v2_dist  (float ax, float ay, float bx, float by) { return lich_v2_len(bx-ax, by-ay); }

LICH_INLINE float lich_v2_norm(float x, float y, float *ox, float *oy) {
    float len = lich_v2_len(x, y);
    float inv = (len > 1e-8f) ? (1.0f / len) : 0.0f;
    if (ox) *ox = x * inv;
    if (oy) *oy = y * inv;
    return len;
}

LICH_INLINE void lich_v2_rot(float x, float y, float ang, float *ox, float *oy) {
    float s, c;
    lich_sincosf(ang, &s, &c);
    if (ox) *ox = x*c - y*s;
    if (oy) *oy = x*s + y*c;
}

LICH_INLINE void lich_v2_reflect(float dx, float dy, float nx, float ny,
                                  float *ox, float *oy) {
    float k = 2.0f * lich_v2_dot(dx, dy, nx, ny);
    if (ox) *ox = dx - k*nx;
    if (oy) *oy = dy - k*ny;
}

LICH_INLINE LICH_PURE float lich_v3_dot(float ax, float ay, float az,
                                         float bx, float by, float bz) {
    return ax*bx + ay*by + az*bz;
}

LICH_INLINE LICH_PURE float lich_v3_len2(float x, float y, float z) { return x*x + y*y + z*z; }
LICH_INLINE LICH_PURE float lich_v3_len (float x, float y, float z) { return lich_sqrtf(x*x + y*y + z*z); }

LICH_INLINE void lich_v3_cross(float ax, float ay, float az,
                                float bx, float by, float bz,
                                float *ox, float *oy, float *oz) {
    if (ox) *ox = ay*bz - az*by;
    if (oy) *oy = az*bx - ax*bz;
    if (oz) *oz = ax*by - ay*bx;
}

LICH_INLINE float lich_v3_norm(float x, float y, float z,
                                float *ox, float *oy, float *oz) {
    float len = lich_v3_len(x, y, z);
    float inv = (len > 1e-8f) ? (1.0f / len) : 0.0f;
    if (ox) *ox = x * inv;
    if (oy) *oy = y * inv;
    if (oz) *oz = z * inv;
    return len;
}

LICH_INLINE void lich_v3_reflect(float dx, float dy, float dz,
                                  float nx, float ny, float nz,
                                  float *ox, float *oy, float *oz) {
    float k = 2.0f * lich_v3_dot(dx, dy, dz, nx, ny, nz);
    if (ox) *ox = dx - k*nx;
    if (oy) *oy = dy - k*ny;
    if (oz) *oz = dz - k*nz;
}

LICH_INLINE void lich_v3_normal_tri(float ax, float ay, float az,
                                     float bx, float by, float bz,
                                     float cx, float cy, float cz,
                                     float *ox, float *oy, float *oz) {
    float ex = bx-ax, ey = by-ay, ez = bz-az;
    float fx = cx-ax, fy = cy-ay, fz = cz-az;
    lich_v3_cross(ex, ey, ez, fx, fy, fz, ox, oy, oz);
}

LICH_INLINE LICH_PURE float lich_edge(float ax, float ay,
                                       float bx, float by,
                                       float px, float py) {
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

LICH_INLINE float lich_bari(float ax, float ay, float bx, float by,
                              float cx, float cy, float px, float py,
                              float *w0, float *w1, float *w2) {
    float area = lich_edge(ax, ay, bx, by, cx, cy);
    if (w0) *w0 = lich_edge(bx, by, cx, cy, px, py);
    if (w1) *w1 = lich_edge(cx, cy, ax, ay, px, py);
    if (w2) *w2 = lich_edge(ax, ay, bx, by, px, py);
    return area;
}

LICH_INLINE LICH_PURE float lich_persp_interp(float va0, float iw0,
                                               float va1, float iw1,
                                               float va2, float iw2,
                                               float b0,  float b1, float b2,
                                               float total_iw) {
    float num = va0*iw0*b0 + va1*iw1*b1 + va2*iw2*b2;
    return (LICH_UNLIKELY(lich_absf(total_iw) < 1e-10f)) ? 0.0f : num / total_iw;
}

LICH_INLINE LICH_PURE float lich_persp_iw_total(float iw0, float iw1, float iw2,
                                                  float b0,  float b1,  float b2) {
    return b0*iw0 + b1*iw1 + b2*iw2;
}

LICH_INLINE LICH_PURE int32_t lich_scan_step_fp(float val0, float val1, int span_len) {
    if (LICH_UNLIKELY(span_len <= 0)) return 0;
    return lich_fp_de_float((val1 - val0) / (float)span_len);
}

LICH_INLINE LICH_PURE uint32_t lich_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
}

LICH_INLINE LICH_PURE uint8_t lich_rgba_r(uint32_t c) { return (uint8_t)(c         & 0xFFu); }
LICH_INLINE LICH_PURE uint8_t lich_rgba_g(uint32_t c) { return (uint8_t)((c >>  8) & 0xFFu); }
LICH_INLINE LICH_PURE uint8_t lich_rgba_b(uint32_t c) { return (uint8_t)((c >> 16) & 0xFFu); }
LICH_INLINE LICH_PURE uint8_t lich_rgba_a(uint32_t c) { return (uint8_t)((c >> 24) & 0xFFu); }

LICH_INLINE LICH_PURE uint32_t lich_rgba_lerp(uint32_t ca, uint32_t cb, uint8_t t) {
    uint32_t it = 255u - (uint32_t)t;
    uint8_t r = (uint8_t)(((uint32_t)lich_rgba_r(ca)*it + (uint32_t)lich_rgba_r(cb)*t) >> 8);
    uint8_t g = (uint8_t)(((uint32_t)lich_rgba_g(ca)*it + (uint32_t)lich_rgba_g(cb)*t) >> 8);
    uint8_t b = (uint8_t)(((uint32_t)lich_rgba_b(ca)*it + (uint32_t)lich_rgba_b(cb)*t) >> 8);
    uint8_t a = (uint8_t)(((uint32_t)lich_rgba_a(ca)*it + (uint32_t)lich_rgba_a(cb)*t) >> 8);
    return lich_rgba(r, g, b, a);
}

LICH_INLINE LICH_PURE uint32_t lich_rgba_mul_alpha(uint32_t c, uint8_t fator) {
    uint8_t r = (uint8_t)(((uint32_t)lich_rgba_r(c) * fator) >> 8);
    uint8_t g = (uint8_t)(((uint32_t)lich_rgba_g(c) * fator) >> 8);
    uint8_t b = (uint8_t)(((uint32_t)lich_rgba_b(c) * fator) >> 8);
    return lich_rgba(r, g, b, lich_rgba_a(c));
}

LICH_INLINE LICH_PURE uint32_t lich_rgba_escurecer(uint32_t c, float fator) {
    uint8_t f = (uint8_t)(uint32_t)lich_clampf(fator * 255.0f, 0.0f, 255.0f);
    return lich_rgba_mul_alpha(c, f);
}

LICH_INLINE LICH_PURE uint32_t lich_rgba_neblina(uint32_t cor, uint32_t cor_fog, float t) {
    uint8_t ti = (uint8_t)(uint32_t)lich_clampf(t * 255.0f, 0.0f, 255.0f);
    return lich_rgba_lerp(cor, cor_fog, ti);
}

LICH_INLINE LICH_PURE uint8_t lich_rgba_lum(uint32_t c) {
    uint32_t r = lich_rgba_r(c);
    uint32_t g = lich_rgba_g(c);
    uint32_t b = lich_rgba_b(c);
    return (uint8_t)((r*77u + g*150u + b*29u) >> 8);
}

LICH_INLINE LICH_PURE uint32_t lich_rgba_cinza(uint32_t c) {
    uint8_t l = lich_rgba_lum(c);
    return lich_rgba(l, l, l, lich_rgba_a(c));
}

LICH_INLINE LICH_PURE uint32_t lich_rgba_blend(uint32_t dst, uint32_t src) {
    uint32_t a  = lich_rgba_a(src);
    uint32_t ia = 255u - a;
    uint8_t r = (uint8_t)(((uint32_t)lich_rgba_r(src)*a + (uint32_t)lich_rgba_r(dst)*ia) >> 8);
    uint8_t g = (uint8_t)(((uint32_t)lich_rgba_g(src)*a + (uint32_t)lich_rgba_g(dst)*ia) >> 8);
    uint8_t b = (uint8_t)(((uint32_t)lich_rgba_b(src)*a + (uint32_t)lich_rgba_b(dst)*ia) >> 8);
    return lich_rgba(r, g, b, lich_rgba_a(dst));
}

LICH_INLINE LICH_PURE float lich_luz_difusa(float nx, float ny, float nz,
                                             float lx, float ly, float lz) {
    float d = lich_v3_dot(nx, ny, nz, lx, ly, lz);
    return d < 0.0f ? 0.0f : d;
}

LICH_INLINE float lich_luz_especular_blinn(float nx, float ny, float nz,
                                            float hx, float hy, float hz,
                                            float exp) {
    float d = lich_v3_dot(nx, ny, nz, hx, hy, hz);
    if (d <= 0.0f) return 0.0f;
    return lich_powf(d, exp);
}

LICH_INLINE void lich_luz_half(float lx, float ly, float lz,
                                float vx, float vy, float vz,
                                float *hx, float *hy, float *hz) {
    float sx = lx+vx, sy = ly+vy, sz = lz+vz;
    lich_v3_norm(sx, sy, sz, hx, hy, hz);
}

LICH_INLINE float lich_luz_sombrear(float ambiente,
                                     float difusa,    float fator_dif,
                                     float especular, float fator_esp) {
    return ambiente + difusa*fator_dif + especular*fator_esp;
}

LICH_INLINE LICH_PURE uint32_t lich_rgba_iluminar(uint32_t c, float intensidade) {
    float f    = lich_clampf(intensidade, 0.0f, 1.0f);
    uint8_t fi = (uint8_t)(uint32_t)(f * 255.0f);
    return lich_rgba_mul_alpha(c, fi);
}

LICH_INLINE LICH_PURE float lich_z_ndc_para_linear(float z_ndc, float near, float far) {
    return (2.0f * near * far) / (far + near - z_ndc * (far - near));
}

LICH_INLINE LICH_PURE float lich_z_linear_para_ndc(float z_lin, float near, float far) {
    if (LICH_UNLIKELY(lich_absf(far - near) < 1e-8f)) return 0.0f;
    return (far + near - 2.0f*near*far / z_lin) / (far - near);
}

LICH_INLINE LICH_PURE float lich_fog_linear(float z, float inicio, float fim) {
    if (LICH_UNLIKELY(fim <= inicio)) return 1.0f;
    return lich_clampf((z - inicio) / (fim - inicio), 0.0f, 1.0f);
}

LICH_INLINE float lich_fog_exp2(float z, float densidade) {
    float e = z * densidade;
    return 1.0f - lich_clampf(lich_powf(2.0f, -(e*e * 1.4426950f)), 0.0f, 1.0f);
}

LICH_INLINE uint32_t lich_neblina_pixel(uint32_t cor_surf, uint32_t cor_fog,
                                         float z, float inicio, float fim) {
    float t = lich_fog_linear(z, inicio, fim);
    return lich_rgba_neblina(cor_surf, cor_fog, t);
}

#endif