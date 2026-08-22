/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: rbtree.h
 *
 */

/* Red - Black trees */

/* El include va deliberadamente FUERA del guard: core.hxx incluye este
   fichero y despues expande DeclSt/DeclPt, que llaman a estas funciones.
   Si la unidad de traduccion entra por aqui y el guard bloquease esa
   re-inclusion, core.hxx veria declaraciones implicitas. Ver NAP-006. */
#include "core.hxx"

#ifndef __CORE_RBTREE_H__
#define __CORE_RBTREE_H__

__EXTERN_C

_core_api RBTree *rbtree_create(FPtr_compare func_compare, const uint16_t esize, const uint16_t ksize, const char_t *type, const char_t *ktype);

_core_api void rbtree_destroy(RBTree **tree, FPtr_remove func_remove, FPtr_destroy func_destroy_key, const char_t *type);

_core_api void rbtree_destroy_ptr(RBTree **tree, FPtr_destroy func_destroy, FPtr_destroy func_destroy_key, const char_t *type);

_core_api uint32_t rbtree_size(const RBTree *tree);

_core_api byte_t *rbtree_get(const RBTree *tree, const void *key, const bool_t isptr, const char_t *ktype);

_core_api byte_t *rbtree_insert(RBTree *tree, const void *key, FPtr_copy func_key_copy, const char_t *ktype);

_core_api bool_t rbtree_insert_ptr(RBTree *tree, const void *key, void *ptr, const char_t *ktype);

_core_api bool_t rbtree_delete(RBTree *tree, const void *key, FPtr_remove func_remove, FPtr_destroy func_destroy_key, const char_t *ktype);

_core_api bool_t rbtree_delete_ptr(RBTree *tree, const void *key, FPtr_destroy func_destroy, FPtr_destroy func_destroy_key, const char_t *ktype);

_core_api byte_t *rbtree_first(RBTree *tree);

_core_api byte_t *rbtree_last(RBTree *tree);

_core_api byte_t *rbtree_next(RBTree *tree);

_core_api byte_t *rbtree_prev(RBTree *tree);

_core_api byte_t *rbtree_first_ptr(RBTree *tree);

_core_api byte_t *rbtree_last_ptr(RBTree *tree);

_core_api byte_t *rbtree_next_ptr(RBTree *tree);

_core_api byte_t *rbtree_prev_ptr(RBTree *tree);

_core_api const char_t *rbtree_get_key(const RBTree *tree);

_core_api bool_t rbtree_check(const RBTree *tree);

__END_C

#endif /* __CORE_RBTREE_H__ */
