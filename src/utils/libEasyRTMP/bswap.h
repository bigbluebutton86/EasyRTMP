/*
 * bswap.h
 *
 *  Created on: 13 окт. 2009
 *      Author: tipok
 */

#ifndef BSWAP_H_
#define BSWAP_H_

#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ASM_OPTIMISATIONS

#if   ARCH_ARM

#ifdef __ARMCC_VERSION

#if HAVE_ARMV6
#define bswap_16 bswap_16
static inline uint16_t bswap_16(unsigned x)
{
    __asm { rev16 x, x }
    return x;
}

#define bswap_32 bswap_32
static inline uint32_t bswap_32(uint32_t x)
{
    return __rev(x);
}
#endif /* HAVE_ARMV6 */

#elif HAVE_INLINE_ASM

#if HAVE_ARMV6
#define bswap_16 bswap_16
static inline uint16_t bswap_16(unsigned x)
{
    __asm__("rev16 %0, %0" : "+r"(x));
    return x;
}
#endif

#define bswap_32 bswap_32
static inline uint32_t bswap_32(uint32_t x)
{
#if HAVE_ARMV6
    __asm__("rev %0, %0" : "+r"(x));
#else
    uint32_t t;
    __asm__ ("eor %1, %0, %0, ror #16 \n\t"
             "bic %1, %1, #0xFF0000   \n\t"
             "mov %0, %0, ror #8      \n\t"
             "eor %0, %0, %1, lsr #8  \n\t"
             : "+r"(x), "=&r"(t));
#endif /* HAVE_ARMV6 */
    return x;
}

#endif /* __ARMCC_VERSION */

#elif ARCH_AVR32
/* TODO */
#elif ARCH_BFIN
/* TODO */
#elif ARCH_SH4
/* TODO */
#elif ARCH_X86

#define bswap_16 bswap_16
static inline uint16_t bswap_16(uint16_t x)
{
    __asm__("rorw $8, %0" : "+r"(x));
    return x;
}

#define bswap_32 bswap_32
static inline uint32_t bswap_32(uint32_t x)
{
#if HAVE_BSWAP
    __asm__("bswap   %0" : "+r" (x));
#else
    __asm__("rorw    $8,  %w0 \n\t"
            "rorl    $16, %0  \n\t"
            "rorw    $8,  %w0"
            : "+r"(x));
#endif
    return x;
}

#if ARCH_X86_64
#define bswap_64 bswap_64
static inline uint64_t bswap_64(uint64_t x)
{
    __asm__("bswap  %0": "=r" (x) : "0" (x));
    return x;
}
#endif


#endif /* x86 */
#endif /* asm optimisations */

#ifndef bswap_16
static inline uint16_t bswap_16(uint16_t x)
{
    x= (x>>8) | (x<<8);
    return x;
}
#endif

#ifndef bswap_32
static inline uint32_t bswap_32(uint32_t x)
{
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    x= (x>>16) | (x<<16);
    return x;
}
#endif

#ifndef bswap_64
static inline uint64_t bswap_64(uint64_t x)
{
#if 0
    x= ((x<< 8)&0xFF00FF00FF00FF00ULL) | ((x>> 8)&0x00FF00FF00FF00FFULL);
    x= ((x<<16)&0xFFFF0000FFFF0000ULL) | ((x>>16)&0x0000FFFF0000FFFFULL);
    return (x>>32) | (x<<32);
#else
    union {
        uint64_t ll;
        uint32_t l[2];
    } w, r;
    w.ll = x;
    r.l[0] = bswap_32 (w.l[1]);
    r.l[1] = bswap_32 (w.l[0]);
    return r.ll;
#endif
}
#endif

// be2me ... big-endian to machine-endian
// le2me ... little-endian to machine-endian

#if HAVE_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#define me2le_16(x) bswap_16(x)
#define me2le_32(x) bswap_32(x)
#define me2le_64(x) bswap_64(x)
#define me2be_16(x) (x)
#define me2be_32(x) (x)
#define me2be_64(x) (x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#define me2be_16(x) bswap_16(x)
#define me2be_32(x) bswap_32(x)
#define me2be_64(x) bswap_64(x)
#define me2le_16(x) (x)
#define me2le_32(x) (x)
#define me2le_64(x) (x)
#endif


#ifndef beptr2me_32
static inline uint32_t beptr2me_32(uint8_t *x)
{
//#if (defined ARCH_ARM) && !(defined HAVE_ARMV6)
    return be2me_32(x[3] << 24 | x[2] << 16 | x[1] << 8 | x[0]);
//#else
//    return be2me_32(*(uint32_t *) x);
//#endif
}
#endif

#ifndef beptr2me_16
static inline uint16_t beptr2me_16(uint8_t *x)
{
//#if (defined ARCH_ARM) && !(defined HAVE_ARMV6)
    return be2me_16(x[1] << 8 | x[0]);
//#else
//    return be2me_16(*(uint16_t *) x);
//#endif
}
#endif


#endif /* BSWAP_H_ */
