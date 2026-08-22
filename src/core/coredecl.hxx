/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: coredecl.hxx
 * https://nappgui.com/en/core/core.html
 *
 */

/* Type-checked container declarations */

/* 'core.hxx' only declares types, so it can be included from the container
   headers without creating a cycle. This header is the other half: it pulls
   the container API ('array.h', 'rbtree.h', 'ntree.h') plus the macro
   generators, defines 'DeclSt'/'DeclPt' and expands them for the basic types.
   Include it (directly or through 'coreall.h') in any translation unit that
   uses the type-checked containers or that expands 'DeclSt'/'DeclPt' for its
   own types. */

#ifndef __CORE_COREDECL_HXX__
#define __CORE_COREDECL_HXX__

#include "core.hxx"
#include "array.h"
#include "rbtree.h"
#include "ntree.h"
#include "arrst.hxx"
#include "arrpt.hxx"
#include "setst.hxx"
#include "setpt.hxx"
#include "treest.hxx"
#include "treept.hxx"

#define DeclSt(type) \
    ArrStDebug(type); \
    SetStDebug(type); \
    TreeStDebug(type); \
    ArrStFuncs(type); \
    SetStFuncs(type); \
    TreeStFuncs(type)

#define DeclPt(type) \
    ArrPtDebug(type); \
    SetPtDebug(type); \
    TreePtDebug(type); \
    ArrPtFuncs(type); \
    SetPtFuncs(type); \
    TreePtFuncs(type)

DeclSt(bool_t);
DeclSt(int8_t);
DeclSt(int16_t);
DeclSt(int32_t);
DeclSt(int64_t);
DeclSt(uint8_t);
DeclSt(uint16_t);
DeclSt(uint32_t);
DeclSt(uint64_t);
DeclSt(real32_t);
DeclSt(real64_t);
DeclPt(String);
DeclSt(DirEntry);
DeclPt(ResPack);
DeclPt(RegEx);

#endif /* __CORE_COREDECL_HXX__ */
