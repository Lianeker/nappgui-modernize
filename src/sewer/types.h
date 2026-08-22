/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: types.h
 *
 */

/* Basic types utils */

#ifndef __SEWER_TYPES_H__
#define __SEWER_TYPES_H__

#include "sewer.hxx"

/* Ojo: evalua x e y dos veces. Prefiere max_u32 / bmath_maxf. */
#define max_val(x, y) (((x) > (y)) ? (x) : (y))

/* Ojo: evalua x e y dos veces. Prefiere min_u32 / bmath_minf. */
#define min_val(x, y) (((x) < (y)) ? (x) : (y))

__EXTERN_C

_sewer_api uint32_t min_u32(const uint32_t v1, const uint32_t v2);

_sewer_api real32_t min_r32(const real32_t v1, const real32_t v2);

_sewer_api real64_t min_r64(const real64_t v1, const real64_t v2);

_sewer_api uint32_t max_u32(const uint32_t v1, const uint32_t v2);

_sewer_api real32_t max_r32(const real32_t v1, const real32_t v2);

_sewer_api real64_t max_r64(const real64_t v1, const real64_t v2);

_sewer_api real32_t abs_r32(const real32_t v);

_sewer_api uint8_t to_u8(const uint32_t v);

__END_C

#endif /* __SEWER_TYPES_H__ */
