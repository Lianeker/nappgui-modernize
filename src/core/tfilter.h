/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tfilter.h
 *
 */

/* Text filters */

#ifndef __CORE_TFILTER_H__
#define __CORE_TFILTER_H__

#include "core.hxx"

__EXTERN_C

_core_api void tfilter_date(const char_t *src, char_t *dest, const uint32_t size, const char_t *pattern, const uint32_t cpos, const bool_t deleted, uint32_t *ncpos);

/*
 * 'tfilter_to_date' reads the date that 'text' holds, taking the position of
 * the day, the month and the year from 'pattern': every 'd'/'D' in the pattern
 * marks a digit of the day, every 'm'/'M' one of the month and every 'y'/'Y'
 * one of the year. Any other character of the pattern is a separator and is
 * ignored, so "dd/mm/yyyy" and "dd-mm-yyyy" read the same text apart from the
 * separator.
 *
 * The function returns 'kDATE_NULL' when it cannot read a date, which the
 * caller detects with 'date_is_null' ('core/date.h'). That happens when:
 *
 * - 'text' and 'pattern' do not have the same length. The filter works in
 *   overwrite mode, so a half typed field keeps the length of the pattern and
 *   only a shorter or longer text gets here.
 * - Any of the three fields is not a number: "1a/02/2020" with "dd/mm/yyyy",
 *   or a pattern without one of the three fields, which cannot give a date.
 *
 * A text whose three fields are numbers is *not* validated as a calendar date:
 * "31/02/2020" is read as day 31 of month 2. Check the result with
 * 'date_is_valid' before using it.
 *
 * A year below 100 is completed as a two digit year: 40 to 99 go to the 20th
 * century (1940-1999) and 0 to 39 to the 21st (2000-2039).
 */
_core_api Date tfilter_to_date(const char_t *text, const char_t *pattern);

__END_C

#endif /* __CORE_TFILTER_H__ */
