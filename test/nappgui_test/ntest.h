/*
 * NAppGUI Cross-platform C SDK
 * MIT Licence
 *
 * File: ntest.h
 *
 */

/* Comprobaciones minimas para la suite de tests.
 *
 * Deliberadamente sin dependencias externas: el SDK presume de no tener
 * ninguna y la suite no va a ser la excepcion. C90 estricto, como el resto.
 */

#ifndef __NTEST_H__
#define __NTEST_H__

#include <sewer/sewer.hxx>

__EXTERN_C

/* Arranca un grupo de comprobaciones. */
void ntest_begin(const char_t *modulo);

/* Cierra el grupo. Devuelve el numero de fallos acumulados. */
uint32_t ntest_end(void);

/* Registra el resultado de una comprobacion. Devuelve `cond` para poder
   encadenar: if (ntest_true(p != NULL, "...")) { ... } */
bool_t ntest_true_imp(const bool_t cond, const char_t *que, const char_t *file, const uint32_t line);

void ntest_equ_u32_imp(const uint32_t obtenido, const uint32_t esperado, const char_t *que, const char_t *file, const uint32_t line);

void ntest_equ_i32_imp(const int32_t obtenido, const int32_t esperado, const char_t *que, const char_t *file, const uint32_t line);

void ntest_equ_r32_imp(const real32_t obtenido, const real32_t esperado, const real32_t tol, const char_t *que, const char_t *file, const uint32_t line);

void ntest_equ_str_imp(const char_t *obtenido, const char_t *esperado, const char_t *que, const char_t *file, const uint32_t line);

/* Documenta un defecto conocido del SDK: se lista al final pero no cuenta
   como fallo, para que la suite pueda estar en verde sin ocultar el problema.
   Siempre con la referencia de la tarea que lo arreglara. */
void ntest_pending_imp(const char_t *que, const char_t *tarea, const char_t *file, const uint32_t line);

__END_C

#define ntest_true(cond, que) \
    ntest_true_imp((cond) ? TRUE : FALSE, que, __FILE__, (uint32_t)__LINE__)

#define ntest_false(cond, que) \
    ntest_true_imp((cond) ? FALSE : TRUE, que, __FILE__, (uint32_t)__LINE__)

#define ntest_equ_u32(obtenido, esperado, que) \
    ntest_equ_u32_imp((uint32_t)(obtenido), (uint32_t)(esperado), que, __FILE__, (uint32_t)__LINE__)

#define ntest_equ_i32(obtenido, esperado, que) \
    ntest_equ_i32_imp((int32_t)(obtenido), (int32_t)(esperado), que, __FILE__, (uint32_t)__LINE__)

#define ntest_equ_r32(obtenido, esperado, que) \
    ntest_equ_r32_imp((real32_t)(obtenido), (real32_t)(esperado), 0.0001f, que, __FILE__, (uint32_t)__LINE__)

#define ntest_near_r32(obtenido, esperado, tol, que) \
    ntest_equ_r32_imp((real32_t)(obtenido), (real32_t)(esperado), (real32_t)(tol), que, __FILE__, (uint32_t)__LINE__)

#define ntest_equ_str(obtenido, esperado, que) \
    ntest_equ_str_imp(obtenido, esperado, que, __FILE__, (uint32_t)__LINE__)

#define ntest_pending(que, tarea) \
    ntest_pending_imp(que, tarea, __FILE__, (uint32_t)__LINE__)

/* Cada modulo expone una funcion con esta forma. */
#define NTEST_DECL(modulo) uint32_t ntest_##modulo(void)

__EXTERN_C
NTEST_DECL(sewer);
NTEST_DECL(core);
NTEST_DECL(geom2d);
NTEST_DECL(encode);
__END_C

#endif /* __NTEST_H__ */
